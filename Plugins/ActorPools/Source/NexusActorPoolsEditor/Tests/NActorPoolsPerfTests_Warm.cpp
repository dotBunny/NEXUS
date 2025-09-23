// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NActorPool.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_PERF(FNActorPoolPerfTests_Warm, "NEXUS::PerfTests::NActorPools::Warm", N_TEST_CONTEXT_EDITOR)
{
	// PRE GC FORCE
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	
	constexpr int32 TestSize = 1000;
	FNTestUtils::WorldTest(EWorldType::Editor, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::APF_ReturnToStorageLocation | ENActorPoolFlags::APF_DeferConstruction | ENActorPoolFlags::APF_ShouldFinishSpawning);
		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);

		//TEST
		{
			N_TEST_TIMER_SCOPE(Warm_Actor_1000, 40.0f)

			Pool.Prewarm(TestSize);

			// Explicitly stop the timer
			Timer.ManualStop();
		}
			
		Pool.Clear();
	}, true);

	// POST GC FORCE
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
}

#endif //WITH_TESTS