// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

DECLARE_LOG_CATEGORY_EXTERN(LogNexusActorPools, Log, All);

DECLARE_STATS_GROUP(TEXT("NActorPools"), STATGROUP_NActorPools, STATCAT_Advanced)
DECLARE_CYCLE_STAT_EXTERN(TEXT("Subsystem Tick"), STAT_ActorPoolSubsystemTick, STATGROUP_NActorPools, NEXUSACTORPOOLS_API)
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Pool Count"), STAT_ActorPoolCount, STATGROUP_NActorPools, NEXUSACTORPOOLS_API)
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("In Actors"), STAT_InActors, STATGROUP_NActorPools, NEXUSACTORPOOLS_API)
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Out Actors"), STAT_OutActors, STATGROUP_NActorPools, NEXUSACTORPOOLS_API)