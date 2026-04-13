// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NActorPool.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::ActorPoolTests::Warm
{
	constexpr int32 ObjectCount = 1000;
	constexpr float PrewarmMaxDuration = 20.0f;
}

N_TEST_PERF(FNActorPoolPerfTests_Warm, 
	"NEXUS::PerfTests::NActorPools::Warm", 
	N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::PrePerformanceTest();
	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage | ENActorPoolFlags::DeferConstruction | ENActorPoolFlags::ShouldFinishSpawning);
		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);

		//TEST
		{
			N_TEST_TIMER_SCOPE(FNActorPoolPerfTests_Warm, NEXUS::ActorPoolTests::Warm::PrewarmMaxDuration)

			Pool.Prewarm(NEXUS::ActorPoolTests::Warm::ObjectCount);

			// Explicitly stop the timer
			NTestTimer.ManualStop();
		}
			
		Pool.Clear();
	}, true);
	FNTestUtils::PostPerformanceTest();
}

#endif //WITH_TESTS