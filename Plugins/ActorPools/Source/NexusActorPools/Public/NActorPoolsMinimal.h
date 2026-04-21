// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/** Log category used by all NexusActorPools logging. */
NEXUSACTORPOOLS_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusActorPools, Log, All);

DECLARE_STATS_GROUP(TEXT("NActorPools"), STATGROUP_NActorPools, STATCAT_Advanced)
DECLARE_CYCLE_STAT_EXTERN(TEXT("Subsystem Tick"), STAT_ActorPoolSubsystemTick, STATGROUP_NActorPools, NEXUSACTORPOOLS_API)
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Pool Count"), STAT_ActorPoolCount, STATGROUP_NActorPools, NEXUSACTORPOOLS_API)
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("In Actors"), STAT_InActors, STATGROUP_NActorPools, NEXUSACTORPOOLS_API)
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Out Actors"), STAT_OutActors, STATGROUP_NActorPools, NEXUSACTORPOOLS_API)


namespace NEXUS::ActorPools
{
	/**
	 * Visual-logger draw constants used when the actor-pool subsystem visualizes spawn requests.
	 */
	namespace VLog
	{
		/** Color used to draw the originally requested spawn point. */
		const FColor RequestedPointColor = FColor::Green;
		/** Color used to draw the actual spawn point after any adjustments. */
		const FColor ActualPointColor = FColor::Blue;
		/** Size of the marker drawn at each logged point. */
		constexpr float PointSize = 2.f;
	}
}
