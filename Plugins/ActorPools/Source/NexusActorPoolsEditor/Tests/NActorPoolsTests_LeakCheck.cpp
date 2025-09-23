// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NActorPool.h"
#include "Developer/NDeveloperUtils.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST(FNActorPoolTests_LeakCheck_DontForceDestroy, "NEXUS::UnitTests::NActorPools::LeakCheck::DontForceDestroy", N_TEST_CONTEXT_EDITOR)
{
	// Tests for leaks when a pool is filled and created, but we do not force the destruction of the created objects.
	FNTestUtils::WorldTestChecked(EWorldType::Editor, [this](UWorld* World)
	{
		const int32 PrePoolObjects = FNDeveloperUtils::GetCurrentObjectCount();
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::APF_ReturnToStorageLocation | ENActorPoolFlags::APF_DeferConstruction | ENActorPoolFlags::APF_ShouldFinishSpawning);
		FNActorPool Pool = FNActorPool(World, AActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();
		
		const int32 PostPoolObjects = FNDeveloperUtils::GetCurrentObjectCount();
		CHECK_EQUALS("Check that no additional objects were created when filling the pool.", PostPoolObjects, PrePoolObjects+Pool.GetSettings().MinimumActorCount);
		
		const int32 PooledActorCount =  Pool.GetInCount();
		CHECK_EQUALS("Check that the expected number of Actors were made.", PooledActorCount, Pool.GetSettings().MinimumActorCount);

		Pool.Clear(false);
	});
}
N_TEST(FNActorPoolTests_LeakCheck, "NEXUS::UnitTests::NActorPools::LeakCheck::ForceDestroy", N_TEST_CONTEXT_EDITOR)
{
	// Tests for leaks when a pool is filled and created, but we let GC run.
	FNTestUtils::WorldTest(EWorldType::Editor, [this](UWorld* World)
	{
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		const int32 PrePoolObjects = FNDeveloperUtils::GetCurrentObjectCount();
		
		FNActorPool Pool = FNActorPool(World, AActor::StaticClass());
		Pool.Fill();
		Pool.Clear(true);
	
		// Force destroy tells the actors to be destroyed; it doesn't mean we actually will have them cleaned up unless we do a GC pass
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		const int32 PostClearObjects = FNDeveloperUtils::GetCurrentObjectCount();
		
		CHECK_EQUALS("Check the number of objects matches the previous count when forcibly destroying the pool.", PostClearObjects, PrePoolObjects);
	}, true);
}

#endif //WITH_TESTS