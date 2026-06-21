// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NActorUtils.h"
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

	/** @return The shared filter settings used to gather collision-source actors from the world. */
	static FNWorldActorFilterSettings CreateWorldActorFilterSettings(const FNWorldAssemblyWorldCollisionSettings& Settings)
	{
		// Collect the world AActors that we need to care about
		FNWorldActorFilterSettings ActorFilterSettings;

		ActorFilterSettings.bExcludeNonCollisionEnabledActors = Settings.bExcludeNonCollisionEnabledActors;
		ActorFilterSettings.bIncludePlayerStarts = Settings.bIncludePlayerStarts;
		ActorFilterSettings.WorldCollisionActorIgnoreTags = Settings.ActorIgnoreTags;

		ActorFilterSettings.ExclusionFunction = &IsWorldCollisionSource;
		return ActorFilterSettings;
	}

	/**
	 * Actor filter for world-collision gathering.
	 * @param Actor Candidate actor under inspection.
	 * @return false to exclude organ volumes (they are inputs, not obstacles); true otherwise.
	 */
	static bool IsWorldCollisionSource(const AActor* Actor)
	{
		// Check global ignore tag
		if (Actor->ActorHasTag(NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore)) return false;

		// We are going to outright ignore volumes as collision data
		if (Actor->IsA<AVolume>()) return false;

		// Nor should any of our debug actors
		if (Actor->IsA<ANDebugActor>()) return false;

		return true;
	}

private:
	/** Virtual-world context being populated. */
	TSharedRef<FNVirtualWorldContext> VirtualWorldContextPtr;

	N_ASSEMBLY_ANALYTICS_SHARED_REF
};