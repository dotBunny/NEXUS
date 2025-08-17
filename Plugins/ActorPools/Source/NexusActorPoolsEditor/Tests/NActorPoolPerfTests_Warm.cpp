// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NActorPool.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_PERF(FNActorPoolPerfTests_Warm, "NEXUS::PerfTests::NActorPools::Warm", N_TEST_CONTEXT_EDITOR)
{
	constexpr int TestSize = 1000;
	FNTestUtils::WorldTest(EWorldType::Editor, [this](UWorld* World)
	{
		FNActorPool Pool = FNActorPool(World, AActor::StaticClass());
		Pool.Fill();

		//TEST
		{
			N_TEST_TIMER_SCOPE(Warm_Actor_1000, 40.0f)
			Pool.Warm(TestSize);
		}
			
		Pool.Clear();
	}, true);
}

#endif //WITH_TESTS