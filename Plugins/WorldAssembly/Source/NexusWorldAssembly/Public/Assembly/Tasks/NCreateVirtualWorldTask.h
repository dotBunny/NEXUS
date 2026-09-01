// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NActorUtils.h"
#include "NWorldAssemblyMinimal.h"
#include "Assembly/NAssemblyTaskAnalytics.h"
#include "Assembly/Contexts/NVirtualWorldContext.h"

/**
 * Task-graph job that snapshots the target world into the supplied FNVirtualWorldContext.
 *
 * Runs on the game thread because it walks live AActors to gather their simple-collision meshes
 * and transforms — those queries are not safe to make from a worker thread. Filters out organ
 * volumes (which are inputs to generation, not collision sources) and any non-collision actors.
 */
class FNCreateVirtualWorldTask
{
public:
	explicit FNCreateVirtualWorldTask(const TSharedPtr<FNVirtualWorldContext>& VirtualWorldContextPtr
		N_ASSEMBLY_ANALYTICS_CONSTRUCTOR)
	: VirtualWorldContextPtr(VirtualWorldContextPtr.ToSharedRef()) N_ASSEMBLY_ANALYTICS_INITIALIZER { }

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNCreateVirtualWorldTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

	/** Executed by the task graph: populates the virtual-world context with world collision data. */
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

	/**
	 * Resolve whatever the level's baked collision cache can answer, and report what is left to gather.
	 * @param OutResolvedKeys Receives the keys satisfied from the cache, so the gather can skip re-emitting them.
	 * @param OutMissedBounds Receives the bounds of the organs whose caches could not be used.
	 * @param bOutMissedUnbounded Set when a missed organ is unbounded, making the remaining gather whole-level.
	 * @param bOutAnyMissed Set when at least one organ still has to be gathered.
	 * @param bOutLandscapeResolved Set when the caller need not sample landscape itself — either the cached section
	 *        was current, or landscape is not being captured at all.
	 * @return true when the cache was consulted at all; false when there is none, or it is turned off, or it was
	 *         baked against a different world origin — in which case the caller captures exactly as it always did,
	 *         over the whole operation's bounds.
	 */
	bool TryCaptureFromCache(TSet<uint64>& OutResolvedKeys, TArray<FBoxSphereBounds>& OutMissedBounds,
		bool& bOutMissedUnbounded, bool& bOutAnyMissed, bool& bOutLandscapeResolved) const;

	/**
	 * Sample the landscape surface into the capture by tracing the live physics scene.
	 * @note Only reached when the cached landscape section could not answer. Game-thread only, and the most expensive
	 *       thing this task does — one downward trace per sample, over the whole landscape.
	 */
	void SampleLandscape() const;

	/**
	 * @return The shared filter settings used to gather collision-source actors from the world.
	 * @note ExclusionFunction is deliberately left unset. It is the end-user hook, and nothing this filter needs is
	 *       user policy — everything below is either a project setting or a framework invariant.
	 */
	static FNWorldActorFilterSettings CreateWorldActorFilterSettings(const FNWorldAssemblyWorldCollisionSettings& Settings)
	{
		// Collect the world AActors that we need to care about
		FNWorldActorFilterSettings ActorFilterSettings;

		ActorFilterSettings.bExcludeNonCollisionEnabledActors = Settings.bExcludeNonCollisionEnabledActors;
		ActorFilterSettings.bIncludePlayerStarts = Settings.bIncludePlayerStarts;

		// The markup tag rides along with the user's list rather than getting a flag of its own — it is an ignore tag,
		// and the filter already knows how to test those.
		ActorFilterSettings.WorldCollisionActorIgnoreTags = Settings.ActorIgnoreTags;
		ActorFilterSettings.WorldCollisionActorIgnoreTags.AddUnique(NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore);

		// Mesh Terrain is settled here, where landscape is settled at each gather site as well. That asymmetry is the
		// shape of the two representations rather than an oversight: a Mesh Terrain section carries a UBodySetup, so it
		// arrives through the ordinary geometry gather and dropping the actor is the only way to leave it out, while a
		// landscape has to survive this filter for FNRawMeshFactory::FromLandscapesInBounds to find and sample it at all.
		ActorFilterSettings.bExcludeMeshTerrains = !Settings.bIncludeMeshTerrains;
		ActorFilterSettings.bExcludeLandscapes = !Settings.bIncludeLandscapes;

		// Not settings-driven, and deliberately so. Volumes are generation inputs rather than obstacles, debug actors
		// are diagnostics, and terrain authoring apparatus describes how a terrain is built rather than being a surface
		// to place cells against — a modifier's bounds are its region of influence, measured against a real level as
		// larger than every piece of geometry in it put together. The cell bounds and hull calculations reject all
		// three for the same reasons, and the world view has to agree with them: these settings are what the editor's
		// collision visualizer and penetration cache gather through, so a phantom obstacle admitted here would be drawn
		// as world collision and avoided during assembly.
		ActorFilterSettings.bExcludeVolumes = true;
		ActorFilterSettings.bExcludeDebugActors = true;
		ActorFilterSettings.bExcludeTerrainAuthoring = true;

		return ActorFilterSettings;
	}

	/**
	 * @return The component tags that exclude an individual primitive from a world-collision gather.
	 * @param Settings Collision-capture settings supplying the user's list.
	 * @note Handed to FNRawMeshFactory::FromActorsInBounds, and shaped exactly like the actor list built above: the
	 *       markup tag rides along with the user's entries rather than getting a check of its own.
	 * @remark Shared so the bake and the live gather it falls back to cannot disagree about what is excluded. They
	 *         already share their actor filter for the same reason — a gather that saw different geometry from the
	 *         bake would make every cached organ read as stale forever.
	 */
	static TArray<FName> CreateWorldComponentIgnoreTags(const FNWorldAssemblyWorldCollisionSettings& Settings)
	{
		TArray<FName> ComponentIgnoreTags = Settings.ComponentIgnoreTags;
		ComponentIgnoreTags.AddUnique(NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore);
		return ComponentIgnoreTags;
	}

private:
	/** Virtual-world context being populated. */
	TSharedRef<FNVirtualWorldContext> VirtualWorldContextPtr;

	N_ASSEMBLY_ANALYTICS_SHARED_REF
};