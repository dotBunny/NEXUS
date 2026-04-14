// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NActorPool.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NActorPools::FNActorPool
{
	constexpr int32 ObjectCount = 1000;
	constexpr float ReturnMaxDuration = 2.5f;
	constexpr float PrewarmMaxDuration = 20.0f;
}

N_TEST_PERF(FNActorPoolPerfTests_Return, "NEXUS::PerfTests::NActorPools::FNActorPool::Return",  N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::PrePerformanceTest();
	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);
		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Prewarm(NEXUS::PerfTests::NActorPools::FNActorPool::ObjectCount);
		
		TArray<AActor*> Actors;
		Actors.Reserve(NEXUS::PerfTests::NActorPools::FNActorPool::ObjectCount);
		for (int32 i = 0; i < NEXUS::PerfTests::NActorPools::FNActorPool::ObjectCount; i++)
		{
			Actors.Add(Pool.Spawn(FVector::Zero(), FRotator::ZeroRotator));
		}

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNActorPoolPerfTests_Return, NEXUS::PerfTests::NActorPools::FNActorPool::ReturnMaxDuration);
			for (int32 i = 0; i < NEXUS::PerfTests::NActorPools::FNActorPool::ObjectCount; i++)
			{
				Pool.Return(Actors[i]);
			}
			
			// Explicitly stop the timer
			NTestTimer.ManualStop();
		}

		Pool.Clear();
	}, true);
	FNTestUtils::PostPerformanceTest();
}


N_TEST_PERF(FNActorPoolPerfTests_Warm, "NEXUS::PerfTests::NActorPools::FNActorPool::Warm", N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::PrePerformanceTest();
	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);
		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);

		//TEST
		{
			N_TEST_TIMER_SCOPE(FNActorPoolPerfTests_Warm, NEXUS::PerfTests::NActorPools::FNActorPool::PrewarmMaxDuration)

			Pool.Prewarm(NEXUS::PerfTests::NActorPools::FNActorPool::ObjectCount);

			// Explicitly stop the timer
			NTestTimer.ManualStop();
		}
			
		Pool.Clear();
	}, true);
	FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS