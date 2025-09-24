// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NActorPool.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_PERF(FNActorPoolPerfTests_Return, "NEXUS::PerfTests::NActorPools::Return", N_TEST_CONTEXT_EDITOR)
{
	// PRE GC FORCE
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	
	constexpr int32 TestSize = 1000;
	FNTestUtils::WorldTest(EWorldType::Editor, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::APF_ReturnToStorageLocation | ENActorPoolFlags::APF_DeferConstruction | ENActorPoolFlags::APF_ShouldFinishSpawning);
		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Prewarm(TestSize);
		
		TArray<AActor*> Actors;
		Actors.Reserve(TestSize);
		for (int32 i = 0; i < TestSize; i++)
		{
			Actors.Add(Pool.Spawn(FVector::Zero(), FRotator::ZeroRotator));
		}

		// TEST
		{
			N_TEST_TIMER_SCOPE(Return_Actor_1000, 2.5f)
			for (int32 i = 0; i < TestSize; i++)
			{
				Pool.Return(Actors[i]);
			}
			
			// Explicitly stop the timer
			NTestTimer.ManualStop();
		}

		Pool.Clear();
	}, true);

	// POST GC FORCE
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
}

#endif //WITH_TESTS