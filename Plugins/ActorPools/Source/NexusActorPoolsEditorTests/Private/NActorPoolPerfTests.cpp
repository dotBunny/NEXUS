// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NActorPool.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::PerfTests::NActorPools::FNActorPoolHarness
{
	constexpr int32 ObjectCount = 1000;
	constexpr float ReturnMaxDuration = 2.5f;
	constexpr float PrewarmMaxDuration = 20.0f;
}

class FNActorPoolPerfTests
{
public:
	static void Return(UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction);
		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Prewarm(NEXUS::PerfTests::NActorPools::FNActorPoolHarness::ObjectCount);

		TArray<AActor*> Actors;
		Actors.Reserve(NEXUS::PerfTests::NActorPools::FNActorPoolHarness::ObjectCount);
		for (int32 i = 0; i < NEXUS::PerfTests::NActorPools::FNActorPoolHarness::ObjectCount; ++i)
		{
			Actors.Add(Pool.Spawn(FVector::Zero(), FRotator::ZeroRotator));
		}

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNActorPoolPerfTests_Return,
				NEXUS::PerfTests::NActorPools::FNActorPoolHarness::ReturnMaxDuration)
			for (int32 i = 0; i < NEXUS::PerfTests::NActorPools::FNActorPoolHarness::ObjectCount; ++i)
			{
				Pool.Return(Actors[i]);
			}
			NTestTimer.ManualStop();
		}

		Pool.Clear();
	}

	static void Warm(UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction);
		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);

		// TEST
		{
			N_TEST_TIMER_SCOPE(FNActorPoolPerfTests_Warm,
				NEXUS::PerfTests::NActorPools::FNActorPoolHarness::PrewarmMaxDuration)
			Pool.Prewarm(NEXUS::PerfTests::NActorPools::FNActorPoolHarness::ObjectCount);
			NTestTimer.ManualStop();
		}

		Pool.Clear();
	}
};

N_TEST_PERF(FNActorPoolPerfTests_Return,
	"NEXUS::PerfTests::NActorPools::FNActorPool::Return",
	N_TEST_CONTEXT_EDITOR)
{
	N_TESTS_PERF_START_LATENT_TEST_WORLD
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_WorldTest(&FNActorPoolPerfTests::Return, true));
	N_TESTS_PERF_FINISH_LATENT_TEST_WORLD
}

N_TEST_PERF(FNActorPoolPerfTests_Warm,
	"NEXUS::PerfTests::NActorPools::FNActorPool::Warm",
	N_TEST_CONTEXT_EDITOR)
{
	N_TESTS_PERF_START_LATENT_TEST_WORLD
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_WorldTest(&FNActorPoolPerfTests::Warm, true));
	N_TESTS_PERF_FINISH_LATENT_TEST_WORLD
}

#endif //WITH_TESTS
