// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NActorPool.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_PERF(FNActorPoolPerfTests_Return, "NEXUS::PerfTests::NActorPools::Return", N_TEST_CONTEXT_EDITOR)
{
	constexpr int TestSize = 1000;
	FNTestUtils::WorldTest(EWorldType::Editor, [this](UWorld* World)
	{
		FNActorPool Pool = FNActorPool(World, AActor::StaticClass());
		Pool.Warm(TestSize);
		
		TArray<AActor*> Actors;
		Actors.Reserve(TestSize);
		for (int i = 0; i < TestSize; i++)
		{
			Actors.Add(Pool.Spawn(FVector::Zero(), FRotator::ZeroRotator));
		}

		// TEST
		{
			N_TEST_TIMER_SCOPE(Return_Actor_1000, 2.0f)
			for (int i = 0; i < TestSize; i++)
			{
				Pool.Return(Actors[i]);
			}
		}

		Pool.Clear();
	}, true);
	
}

#endif //WITH_TESTS