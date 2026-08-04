// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Tasks/NConnectJunctionsTask.h"

#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblySettings.h"
#include "Assembly/NJunctionConnectorSolver.h"
#include "Types/NRawMeshUtils.h"

FNConnectJunctionsTask::FNConnectJunctionsTask(const TSharedPtr<FNVirtualWorldContext>& WorldContextPtr,
	const TSharedPtr<FNAssemblyTaskGraphContext>& TaskGraphContextPtr, const FVector2D& SocketUnitSize
	N_ASSEMBLY_ANALYTICS_CONSTRUCTOR)
	: WorldContextPtr(WorldContextPtr.ToSharedRef()), TaskGraphContextPtr(TaskGraphContextPtr.ToSharedRef())
	, SocketUnitSize(SocketUnitSize)
	N_ASSEMBLY_ANALYTICS_INITIALIZER
{
}

void FNConnectJunctionsTask::GatherOpenJunctions(TArray<FOpenJunction>& OutJunctions) const
{
	OutJunctions.Reset();

	// The virtual world stores placed-cell hulls in a flat array parallel to NodeIndex; a reverse lookup lets a
	// route exempt its own endpoint cells without re-searching that array per probe.
	TMap<const FNAssemblyGraphCellNode*, int32> NodeCollisionIndices;
	NodeCollisionIndices.Reserve(WorldContextPtr->NodeIndex.Num());
	for (int32 i = 0; i < WorldContextPtr->NodeIndex.Num(); i++)
	{
		NodeCollisionIndices.Add(WorldContextPtr->NodeIndex[i], i);
	}

	const int32 GraphCount = TaskGraphContextPtr->Graphs.Num();
	for (int32 GraphIndex = 0; GraphIndex < GraphCount; GraphIndex++)
	{
		const TUniquePtr<FNAssemblyGraph>& Graph = TaskGraphContextPtr->Graphs[GraphIndex];
		for (FNAssemblyGraphNode* Node : Graph->GetNodes())
		{
			if (Node->GetNodeType() != ENAssemblyGraphNodeType::Cell) continue;

			FNAssemblyGraphCellNode* CellNode = static_cast<FNAssemblyGraphCellNode*>(Node);

			// Sorted so the gather order depends only on the graph's contents, not on the junction map's hashing.
			TArray<int32> JunctionKeys;
			CellNode->GetOpenJunctions().GetKeys(JunctionKeys);
			JunctionKeys.Sort();

			for (const int32 JunctionKey : JunctionKeys)
			{
				const FNCellJunctionDetails* Details = CellNode->GetJunctionDetails(JunctionKey);
				if (Details == nullptr) continue;

				FOpenJunction& Junction = OutJunctions.AddDefaulted_GetRef();
				Junction.GraphIndex = GraphIndex;
				Junction.CellNode = CellNode;
				Junction.JunctionKey = JunctionKey;
				Junction.NodeCollisionIndex = NodeCollisionIndices.FindRef(CellNode, INDEX_NONE);
				Junction.Details = *Details;
			}
		}
	}
}

void FNConnectJunctionsTask::BuildCandidatePairs(const TArray<FOpenJunction>& Junctions, TArray<FCandidatePair>& OutPairs) const
{
	OutPairs.Reset();

	const FNWorldAssemblyJunctionConnectorSettings& Settings = TaskGraphContextPtr->OperationSettings.JunctionConnectorSettings;
	const double MaximumRangeSquared = static_cast<double>(Settings.MaximumRange) * Settings.MaximumRange;

	const int32 JunctionCount = Junctions.Num();

	// Broadphase over the junction points so a dense layout does not pay the full N^2 distance sweep. Built from
	// zero-extent boxes and queried with an expanded one, which keeps the range test on the query side.
	TArray<FBox> JunctionBounds;
	JunctionBounds.Reserve(JunctionCount);
	for (const FOpenJunction& Junction : Junctions)
	{
		JunctionBounds.Add(FBox(Junction.Details.WorldLocation, Junction.Details.WorldLocation));
	}
	const FNBoundsBVH JunctionBVH(JunctionBounds);

	const FVector RangeExtent(Settings.MaximumRange);
	TArray<int32, TInlineAllocator<32>> Overlaps;
	for (int32 StartIndex = 0; StartIndex < JunctionCount; StartIndex++)
	{
		const FOpenJunction& Start = Junctions[StartIndex];
		JunctionBVH.QueryOverlaps(FBox(Start.Details.WorldLocation - RangeExtent, Start.Details.WorldLocation + RangeExtent), Overlaps);

		for (const int32 EndIndex : Overlaps)
		{
			// Each unordered pair is emitted once, from its lower index.
			if (EndIndex <= StartIndex) continue;

			const FOpenJunction& End = Junctions[EndIndex];

			// Two junctions on the same cell face into the same interior; connecting them is never what was meant.
			if (Start.CellNode == End.CellNode) continue;

			// Sockets have to match for a connector to weld cleanly to both openings.
			if (Start.Details.SocketSize != End.Details.SocketSize) continue;

			const double DistanceSquared = FVector::DistSquared(Start.Details.WorldLocation, End.Details.WorldLocation);
			if (DistanceSquared > MaximumRangeSquared) continue;

			FCandidatePair& Pair = OutPairs.AddDefaulted_GetRef();
			Pair.StartIndex = StartIndex;
			Pair.EndIndex = EndIndex;
			Pair.DistanceSquared = DistanceSquared;
		}
	}

	// Nearest-first, so the greedy walk spends each junction on its closest viable partner. The index tiebreak is
	// what keeps two equidistant candidates from resolving differently between runs.
	OutPairs.Sort([&Junctions](const FCandidatePair& Left, const FCandidatePair& Right)
	{
		if (Left.DistanceSquared != Right.DistanceSquared)
		{
			return Left.DistanceSquared < Right.DistanceSquared;
		}
		if (Left.StartIndex != Right.StartIndex)
		{
			return Left.StartIndex < Right.StartIndex;
		}
		return Left.EndIndex < Right.EndIndex;
	});
}

bool FNConnectJunctionsTask::DoesHullCollide(const FNRawMesh& Hull, const int32 IgnoreNodeIndexA, const int32 IgnoreNodeIndexB) const
{
	const FBox HullBounds = Hull.Bounds;

	// World geometry.
	WorldContextPtr->WorldCollisionBVH.QueryOverlaps(HullBounds, OverlapScratch);
	for (const int32 Index : OverlapScratch)
	{
		if (FNRawMeshUtils::DoesIntersect(Hull, WorldContextPtr->WorldCollisionMeshes[Index]))
		{
			return true;
		}
	}

	// Meshes without bounds cannot be broadphased, so the BVH never returns them; they have to be tested every time
	// to stay equivalent to a linear scan. Empty for well-formed input.
	for (const int32 Index : WorldContextPtr->UnboundedWorldCollisionIndices)
	{
		if (FNRawMeshUtils::DoesIntersect(Hull, WorldContextPtr->WorldCollisionMeshes[Index]))
		{
			return true;
		}
	}

	// Placed cells.
	NodeCollisionBVH.QueryOverlaps(HullBounds, OverlapScratch);
	for (const int32 Index : OverlapScratch)
	{
		if (Index == IgnoreNodeIndexA || Index == IgnoreNodeIndexB) continue;

		if (FNRawMeshUtils::DoesIntersect(Hull, WorldContextPtr->NodeCollisionMeshes[Index]))
		{
			return true;
		}
	}

	// Connectors accepted earlier in this same pass. The tree covers a prefix and the tail is scanned, so both parts
	// are walked; the bounds test in front of each keeps the scan cheap.
	WorldContextPtr->ConnectorCollisionBVH.QueryOverlaps(HullBounds, OverlapScratch);
	for (const int32 Index : OverlapScratch)
	{
		if (FNRawMeshUtils::DoesIntersect(Hull, WorldContextPtr->ConnectorCollisionMeshes[Index]))
		{
			return true;
		}
	}
	for (int32 Index = WorldContextPtr->ConnectorCollisionIndexedCount; Index < WorldContextPtr->ConnectorCollisionMeshes.Num(); Index++)
	{
		if (!WorldContextPtr->ConnectorCollisionBounds[Index].Intersect(HullBounds)) continue;

		if (FNRawMeshUtils::DoesIntersect(Hull, WorldContextPtr->ConnectorCollisionMeshes[Index]))
		{
			return true;
		}
	}

	return false;
}

bool FNConnectJunctionsTask::DoesSweepCollide(const TArray<FNRawMesh>& Hulls, const FOpenJunction& StartJunction,
	const FOpenJunction& EndJunction, const float SegmentLength) const
{
	const FNWorldAssemblyJunctionConnectorSettings& Settings = TaskGraphContextPtr->OperationSettings.JunctionConnectorSettings;

	// Convert the exclusion distance into a hull count. Hulls are near enough uniform in length, so counting them is
	// equivalent to measuring along the path and avoids re-walking it.
	const int32 ExclusionHullCount = SegmentLength > UE_SMALL_NUMBER
		? FMath::CeilToInt32(Settings.EndpointExclusion / SegmentLength)
		: 0;

	const int32 HullCount = Hulls.Num();
	for (int32 i = 0; i < HullCount; i++)
	{
		// Each endpoint cell is exempt only near its own socket. Past that the cell is tested again, which is what
		// catches a route that curls back through the cell it started in.
		const int32 IgnoreStart = (i < ExclusionHullCount) ? StartJunction.NodeCollisionIndex : INDEX_NONE;
		const int32 IgnoreEnd = (i >= HullCount - ExclusionHullCount) ? EndJunction.NodeCollisionIndex : INDEX_NONE;

		if (DoesHullCollide(Hulls[i], IgnoreStart, IgnoreEnd))
		{
			return true;
		}
	}

	return false;
}

void FNConnectJunctionsTask::RetainConnectorHulls(const TArray<FNRawMesh>& Hulls)
{
	WorldContextPtr->ConnectorCollisionMeshes.Reserve(WorldContextPtr->ConnectorCollisionMeshes.Num() + Hulls.Num());
	for (const FNRawMesh& Hull : Hulls)
	{
		const int32 Index = WorldContextPtr->ConnectorCollisionMeshes.Add(Hull);
		WorldContextPtr->ConnectorCollisionBounds.Add(Hull.Bounds);

		// The copy lands cold: FNRawMesh's copy constructor drops the face-plane cache and leaves validation dirty.
		// Warming here, on the thread that owns the array, keeps every later probe off the lazy rebuild path.
		const FNRawMesh& Retained = WorldContextPtr->ConnectorCollisionMeshes[Index];
		Retained.EnsureValidated();
		Retained.EnsureCachedFacePlanes();
	}

	WorldContextPtr->EnsureConnectorCollisionIndex();
}

void FNConnectJunctionsTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	N_ASSEMBLY_ANALYTICS(ConnectJunctionsStart)

	const FNWorldAssemblyJunctionConnectorSettings& Settings = TaskGraphContextPtr->OperationSettings.JunctionConnectorSettings;
	if (!Settings.bEnabled || TaskGraphContextPtr->IsCancelled())
	{
		N_ASSEMBLY_ANALYTICS(ConnectJunctionsFinish)
		return;
	}

	FNConnectJunctionsAnalytics Analytics;

	TArray<FOpenJunction> Junctions;
	GatherOpenJunctions(Junctions);
	Analytics.OpenJunctionCount = Junctions.Num();

	// A single open junction has nothing to pair with, and zero means the layout closed itself off entirely.
	if (Junctions.Num() < 2)
	{
		N_ASSEMBLY_ANALYTICS_ONE_PARAM(ConnectJunctions_SetResult, Analytics)
		N_ASSEMBLY_ANALYTICS(ConnectJunctionsFinish)
		return;
	}

	TArray<FCandidatePair> Pairs;
	BuildCandidatePairs(Junctions, Pairs);
	Analytics.CandidatePairCount = Pairs.Num();

	if (Pairs.Num() == 0)
	{
		N_ASSEMBLY_ANALYTICS_ONE_PARAM(ConnectJunctions_SetResult, Analytics)
		N_ASSEMBLY_ANALYTICS(ConnectJunctionsFinish)
		return;
	}

	TaskGraphContextPtr->SetStatusMessage(FString::Printf(TEXT("%s (%i)"),
		*NEXUS::WorldAssembly::StatusMessage::ConnectingJunctions, Pairs.Num()));
	const int32 StatusChannelId = TaskGraphContextPtr->OpenStatusChannel(TEXT("Connecting Junctions"));

	// Broadphase over the placed cells. The virtual world never built one — organ builders query it through their own
	// per-pass snapshots — so it is built here, once, over the now-complete array.
	{
		TArray<FBox> NodeBounds;
		NodeBounds.Reserve(WorldContextPtr->NodeCollisionMeshes.Num());
		for (const FNRawMesh& Mesh : WorldContextPtr->NodeCollisionMeshes)
		{
			NodeBounds.Add(Mesh.HasBounds() ? Mesh.Bounds : FBox(ForceInit));
		}
		NodeCollisionBVH = FNBoundsBVH(NodeBounds);
	}

	TArray<FVector> AvoidanceMidPoints;
	TArray<FNRawMesh> RadiusHulls;
	TArray<FNRawMesh> CornerHulls;
	FNJunctionConnectorRoute Route;

	int32 NextConnectorIdentifier = 0;
	const int32 PairCount = Pairs.Num();
	for (int32 PairIndex = 0; PairIndex < PairCount; PairIndex++)
	{
		if (TaskGraphContextPtr->IsCancelled()) break;

		// Progress is reported per candidate rather than per acceptance: most candidates are rejected, so an
		// acceptance-based bar would sit still through the bulk of the work.
		if ((PairIndex % 16) == 0)
		{
			TaskGraphContextPtr->SetChannelStatus(StatusChannelId,
				FString::Printf(TEXT("Connected %i/%i"), Analytics.AcceptedCount, PairCount),
				static_cast<float>(PairIndex) / PairCount);
		}

		const FCandidatePair& Pair = Pairs[PairIndex];
		FOpenJunction& Start = Junctions[Pair.StartIndex];
		FOpenJunction& End = Junctions[Pair.EndIndex];
		if (Start.bMatched || End.bMatched) continue;

		// One connection per pair of cells, unless the operation opts out. Evaluated here rather than while building
		// the candidate list because the graph gains links as this walk accepts pairings — a pair that was legal when
		// the list was built can be made illegal by an earlier acceptance, and a build-time filter would catch the
		// builder's matings while silently missing every connector this pass created.
		//
		// The junction link map is the authority for both: LinkJunction and LinkJunctionConnector write to the same
		// place, so one lookup answers "are these two cells joined at all" without separate bookkeeping. Only a
		// direct link counts, so cells joined through a chain of others stay connectable.
		if (!Settings.bAllowMultipleCellConnections
			&& (Start.CellNode->FindJunctionKeyLinkedTo(End.CellNode) != INDEX_NONE
				|| End.CellNode->FindJunctionKeyLinkedTo(Start.CellNode) != INDEX_NONE))
		{
			Analytics.RejectedByExistingConnectionCount++;
			continue;
		}

		// Try the direct route first, then progressively larger detours. AvoidanceMidPoints is built lazily because
		// most pairs either clear on the direct route or fail on length, and never need it.
		AvoidanceMidPoints.Reset();
		bool bBuiltAvoidance = false;
		bool bAccepted = false;
		bool bLengthRejected = false;
		int32 Attempt = 0;

		while (true)
		{
			const FVector* MidPoint = (Attempt == 0) ? nullptr : &AvoidanceMidPoints[Attempt - 1];

			if (FNJunctionConnectorSolver::BuildRoute(Start.Details, End.Details, SocketUnitSize, Settings, MidPoint, Route))
			{
				bLengthRejected = false;

				const float SegmentLength = Route.Path.Center.Length / FMath::Max(Route.Path.Center.Points.Num() - 1, 1);

				// Coarse pass first: a fixed-radius tube is cheaper to build and test than the socket-shaped one, and
				// rejects the routes that are plainly buried before the exact hulls are ever constructed.
				FNJunctionConnectorSolver::BuildRadiusHulls(Route, Settings.SplineRadius, RadiusHulls);
				if (!DoesSweepCollide(RadiusHulls, Start, End, SegmentLength))
				{
					FNJunctionConnectorSolver::BuildCornerHulls(Route, CornerHulls);
					if (!DoesSweepCollide(CornerHulls, Start, End, SegmentLength))
					{
						bAccepted = true;
						break;
					}
				}
			}
			else if (Attempt == 0)
			{
				// The direct route already fails on length. A detour only ever adds length, so there is nothing to
				// try — record the reason and move on rather than burning the whole avoidance budget.
				bLengthRejected = true;
				break;
			}

			if (!bBuiltAvoidance)
			{
				FNJunctionConnectorSolver::BuildAvoidanceMidPoints(Start.Details.WorldLocation, End.Details.WorldLocation,
					Settings.AvoidanceOffsetStep, Settings.MaximumAvoidanceAttempts, AvoidanceMidPoints);
				bBuiltAvoidance = true;
			}

			if (Attempt >= AvoidanceMidPoints.Num()) break;

			Attempt++;
			Analytics.AvoidanceAttemptCount++;
		}

		if (!bAccepted)
		{
			if (bLengthRejected)
			{
				Analytics.RejectedByLengthCount++;
			}
			else
			{
				Analytics.RejectedByCollisionCount++;
			}
			continue;
		}

		if (Attempt > 0)
		{
			Analytics.AvoidanceSuccessCount++;
		}

		const int32 ConnectorIdentifier = NextConnectorIdentifier++;

		// Link both ends so GenerateLinkDetails reports them connected — which is what stops either junction being
		// capped off at BeginPlay — and records the pairing identifier that rejoins them at runtime.
		Start.CellNode->LinkJunctionConnector(Start.JunctionKey, End.CellNode, ConnectorIdentifier);
		End.CellNode->LinkJunctionConnector(End.JunctionKey, Start.CellNode, ConnectorIdentifier);

		// Wire the node-level edge too, so the pairing is visible to graph traversal — hot pathing in particular
		// walks upstream/downstream links rather than the junction map, and would otherwise route around a connector
		// that players can walk straight through. Called once, from the deterministically chosen start end, because
		// the edge belongs to the pair rather than to either junction.
		//
		// This can join nodes from two different graphs. Those graphs are destroyed together at operation teardown,
		// and neither ~FNAssemblyGraph nor ~FNAssemblyGraphNode walks neighbour arrays, so the cross-graph pointers
		// left dangling between the two destructions are never dereferenced.
		Start.CellNode->Connect(End.CellNode);

		Start.bMatched = true;
		End.bMatched = true;

		FNCellJunctionConnection& Connection = TaskGraphContextPtr->JunctionConnections.AddDefaulted_GetRef();
		Connection.ConnectorIdentifier = ConnectorIdentifier;
		Connection.OperationTicket = TaskGraphContextPtr->OperationTicket;
		Connection.StartNodeIdentifier = Start.CellNode->GetNodeIdentifier();
		Connection.StartJunctionInstanceIdentifier = Start.Details.InstanceIdentifier;
		Connection.StartOrganIdentifier = TaskGraphContextPtr->Graphs[Start.GraphIndex]->GetOrganIdentifier();
		Connection.StartTransform = FTransform(Start.Details.WorldRotation, Start.Details.WorldLocation);
		Connection.EndNodeIdentifier = End.CellNode->GetNodeIdentifier();
		Connection.EndJunctionInstanceIdentifier = End.Details.InstanceIdentifier;
		Connection.EndOrganIdentifier = TaskGraphContextPtr->Graphs[End.GraphIndex]->GetOrganIdentifier();
		Connection.EndTransform = FTransform(End.Details.WorldRotation, End.Details.WorldLocation);
		Connection.Path = MoveTemp(Route.Path);

		// Retain the exact socket-shaped volume rather than the coarse one: the radius tube is a testing
		// approximation and can be fatter or thinner than the geometry that will actually occupy this space.
		RetainConnectorHulls(CornerHulls);

		Analytics.AcceptedCount++;
	}

	Analytics.ConnectorHullCount = WorldContextPtr->ConnectorCollisionMeshes.Num();

	TaskGraphContextPtr->SetChannelStatus(StatusChannelId,
		FString::Printf(TEXT("Connected %i"), Analytics.AcceptedCount), 1.f);
	TaskGraphContextPtr->CloseStatusChannel(StatusChannelId);

	UE_LOG(LogNexusWorldAssembly, Log,
		TEXT("Junction connectors: %i accepted from %i candidates across %i open junctions (%i rejected on length, %i on collision)."),
		Analytics.AcceptedCount, Analytics.CandidatePairCount, Analytics.OpenJunctionCount,
		Analytics.RejectedByLengthCount, Analytics.RejectedByCollisionCount);

	N_ASSEMBLY_ANALYTICS_ONE_PARAM(ConnectJunctions_SetResult, Analytics)
	N_ASSEMBLY_ANALYTICS(ConnectJunctionsFinish)
}
