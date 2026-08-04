// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"
#include "Assembly/Contexts/NVirtualWorldContext.h"
#include "Assembly/NAssemblyTaskAnalytics.h"
#include "Async/TaskGraphInterfaces.h"

/**
 * Task-graph job that pairs up the junctions the organ builders left unmatched and routes a collision-free path
 * between each pair, so two cells whose openings face each other across clear space can be joined by geometry
 * instead of both being capped off.
 *
 * Runs once, after every pass has collected its graphs and before FNCreateSpawnsTask generates link details — which
 * is what lets an accepted pairing simply link the two junctions and have the existing link-detail generation carry
 * the result to runtime.
 *
 * Pairs are considered nearest-first and accepted greedily, and every accepted route is retained as collision so
 * later pairs route around it. Nothing here consults a random stream: candidate ordering, corner correspondence and
 * detour ordering are all derived from the geometry, so the same layout always connects up the same way.
 *
 * Runs on any worker thread (see GetDesiredThread). It reads the graphs and the virtual world snapshot, both of
 * which are complete and no longer being mutated by the time its prerequisites have finished, and is the only
 * writer of the connector collision arrays and the connection list.
 */
struct FNConnectJunctionsTask
{
	/**
	 * @param WorldContextPtr Virtual world snapshot supplying collision geometry.
	 * @param TaskGraphContextPtr Task-graph context supplying the built graphs and the operation settings.
	 * @param SocketUnitSize World size of a single socket grid unit, snapshotted from UNWorldAssemblySettings by the
	 *        graph builder on the game thread — this task cannot reach a UObject from the worker thread it runs on.
	 */
	explicit FNConnectJunctionsTask(const TSharedPtr<FNVirtualWorldContext>& WorldContextPtr,
		const TSharedPtr<FNAssemblyTaskGraphContext>& TaskGraphContextPtr, const FVector2D& SocketUnitSize
		N_ASSEMBLY_ANALYTICS_CONSTRUCTOR);

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNConnectJunctionsTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyNormalThreadNormalTask; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	/** Executed by the task graph: matches unmatched junctions and records the routes between them. */
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

private:
	/** One unmatched junction, with everything needed to gate, order and route it. */
	struct FOpenJunction
	{
		/** Index of the graph the owning cell belongs to; the first half of the deterministic ordering key. */
		int32 GraphIndex = INDEX_NONE;

		/** The cell node that owns this junction. */
		FNAssemblyGraphCellNode* CellNode = nullptr;

		/** Key of the junction within its cell's junction map. */
		int32 JunctionKey = INDEX_NONE;

		/** Index into the virtual world's node arrays for the owning cell, or INDEX_NONE when it was never registered. */
		int32 NodeCollisionIndex = INDEX_NONE;

		/** Copy of the junction's world-space details, so routing never reaches back into the node. */
		FNCellJunctionDetails Details;

		/** Set once this junction has been paired, so the greedy walk skips it. */
		bool bMatched = false;
	};

	/** One pair that cleared the cheap gates and is worth attempting to route. */
	struct FCandidatePair
	{
		/** Index into the open-junction list of the pair's start end. */
		int32 StartIndex = INDEX_NONE;

		/** Index into the open-junction list of the pair's end. */
		int32 EndIndex = INDEX_NONE;

		/** Straight-line distance between the two sockets, which orders the greedy walk. */
		double DistanceSquared = 0.0;
	};

	/**
	 * Collect every unmatched junction across every graph, in a stable order, skipping those authored to opt out.
	 * @param OutJunctions Receives the junctions this pass may pair. Reset first.
	 * @param OutDisabledCount Receives how many were skipped for having Disable Connecting set.
	 */
	void GatherOpenJunctions(TArray<FOpenJunction>& OutJunctions, int32& OutDisabledCount) const;

	/** Emit each unordered pair that clears the socket-size, distinct-cell and range gates, ordered nearest-first. */
	void BuildCandidatePairs(const TArray<FOpenJunction>& Junctions, TArray<FCandidatePair>& OutPairs) const;

	/**
	 * Test a swept hull against world geometry, placed cells and previously accepted connectors.
	 * @param Hull The probe volume.
	 * @param IgnoreNodeIndexA A placed cell to skip, or INDEX_NONE. Used to let a route leave its own socket.
	 * @param IgnoreNodeIndexB A second placed cell to skip, or INDEX_NONE.
	 * @return true when the hull intersects anything not ignored.
	 */
	bool DoesHullCollide(const FNRawMesh& Hull, int32 IgnoreNodeIndexA, int32 IgnoreNodeIndexB) const;

	/**
	 * Test every hull of a swept route, exempting each endpoint cell near its own socket.
	 *
	 * A socket sits on its cell's hull surface, so a probe there always intersects the cell that owns it. Rather
	 * than exempting the two cells outright — which would let a route tunnel back through its own cell unnoticed —
	 * the exemption is limited to the hulls within the configured endpoint exclusion distance of each socket.
	 * @param Hulls The swept hulls, in path order.
	 * @param StartJunction The junction the route leaves from.
	 * @param EndJunction The junction the route arrives at.
	 * @param SegmentLength Approximate world length each hull spans, used to convert the exclusion distance to a hull count.
	 * @return true when any hull collides.
	 */
	bool DoesSweepCollide(const TArray<FNRawMesh>& Hulls, const FOpenJunction& StartJunction,
		const FOpenJunction& EndJunction, float SegmentLength) const;

	/** Retain an accepted route's hulls so later pairs treat it as solid. */
	void RetainConnectorHulls(const TArray<FNRawMesh>& Hulls);

	/** Virtual world snapshot supplying the collision geometry, and receiving the accepted connector volumes. */
	TSharedRef<FNVirtualWorldContext> WorldContextPtr;

	/** Top-level task-graph context supplying the graphs and receiving the accepted connections. */
	TSharedRef<FNAssemblyTaskGraphContext> TaskGraphContextPtr;

	/** World size of a single socket grid unit, snapshotted on the game thread at construction. */
	FVector2D SocketUnitSize;

	/** Broadphase over the placed-cell hulls in the virtual world context, built once at task start. */
	FNBoundsBVH NodeCollisionBVH;

	/** Scratch overlap results, reused across probes so the broadphase queries settle at zero allocations. */
	mutable TArray<int32, TInlineAllocator<32>> OverlapScratch;

	N_ASSEMBLY_ANALYTICS_SHARED_REF
};
