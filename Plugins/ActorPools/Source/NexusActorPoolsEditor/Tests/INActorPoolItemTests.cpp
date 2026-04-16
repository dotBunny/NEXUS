// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NActorPool.h"
#include "NTestPooledActor.h"
#include "Developer/NTestObject.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST(INActorPoolItemTests_Events,
"NEXUS::UnitTests::NActorPools::INActorPoolItem::Events",
	N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::WorldTest(EWorldType::PIE, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 1;
		ActorPoolSettings.MaximumActorCount = 1;
		ActorPoolSettings.Strategy = ENActorPoolStrategy::Create;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::BroadcastDestroy);

		FNActorPool Pool = FNActorPool(World, ANTestPooledActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		ANTestPooledActor* SpawnedActor = Cast<ANTestPooledActor>(Pool.Spawn(FVector::Zero(), FRotator::ZeroRotator));

		const TUniquePtr<FNTestObject> TestObject = MakeUnique<FNTestObject>();
		FNTestObject* TestObjectPtr = TestObject.Get();
		SpawnedActor->OnActorOperationalStateChanged.AddLambda([this, TestObjectPtr](const ENActorOperationalState OldState, const ENActorOperationalState NewState)
		{
			TestObjectPtr->Counter++;
			TestObjectPtr->State = static_cast<uint8>(NewState);
		});

		SpawnedActor->ReturnToActorPool();

		CHECK_EQUALS("Check return to pool state event occurred.", TestObject->State, static_cast<uint8>(ENActorOperationalState::Disabled))
		CHECK_EQUALS("Delegate should have fired exactly once after return.", TestObject->Counter, 1)
		CHECK_EQUALS("Pool should have 1 actor in after return.", Pool.GetInCount(), 1)
		CHECK_EQUALS("Pool should have 0 actors out after return.", Pool.GetOutCount(), 0)

		Pool.Spawn(FVector::Zero(), FRotator::ZeroRotator);

		CHECK_EQUALS("Check spawn state event occurred.", TestObject->State, static_cast<uint8>(ENActorOperationalState::Enabled))
		CHECK_EQUALS("Delegate should have fired exactly twice after spawn.", TestObject->Counter, 2)
		CHECK_EQUALS("Pool should have 0 actors in after spawn.", Pool.GetInCount(), 0)
		CHECK_EQUALS("Pool should have 1 actor out after spawn.", Pool.GetOutCount(), 1)

		Pool.Clear();

		CHECK_EQUALS("Check destroyed state event occurred.", TestObject->State, static_cast<uint8>(ENActorOperationalState::Destroyed))
		CHECK_EQUALS("Delegate should have fired exactly three times after clear.", TestObject->Counter, 3)
	});
}

#endif //WITH_TESTS
