// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Misc/Timespan.h"
#include "NActorPool.h"
#include "NPooledActor.h"
#include "Developer/NTestObject.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST(FNActorPoolsTests_Interface_Events, "NEXUS::UnitTests::NActorPools::Interface::Events", N_TEST_CONTEXT_EDITOR)
{
	FNTestUtils::WorldTest(EWorldType::Editor, [this](UWorld* World)
	{
		FNActorPoolSettings ActorPoolSettings = FNActorPoolSettings();
		ActorPoolSettings.MinimumActorCount = 1;
		ActorPoolSettings.MaximumActorCount = 1;
		ActorPoolSettings.Strategy = APS_Create;
		ActorPoolSettings.Flags = static_cast<uint8>(ENActorPoolFlags::APF_BroadcastDestroy);
		
		FNActorPool Pool = FNActorPool(World, ANPooledActor::StaticClass(), ActorPoolSettings);
		Pool.Fill();

		ANPooledActor* SpawnedActor = Cast<ANPooledActor>(Pool.Spawn(FVector::Zero(), FRotator::ZeroRotator));

		const TUniquePtr<FNTestObject> TestObject = MakeUnique<FNTestObject>();
		FNTestObject* TestObjectPtr = TestObject.Get();
		SpawnedActor->OnActorOperationalStateChanged.AddLambda([this, TestObjectPtr](const ENActorOperationalState OldState, const ENActorOperationalState NewState)
		{
			TestObjectPtr->Counter++;
			TestObjectPtr->State = NewState;
		});
		
		SpawnedActor->ReturnToActorPool();
		
		CHECK_EQUALS("Check return to pool state event occured.", TestObject->State, ENActorOperationalState::AOS_Disabled)

		Pool.Spawn(FVector::Zero(), FRotator::ZeroRotator);
		
		CHECK_EQUALS("Check spawn state event occured.", TestObject->State, ENActorOperationalState::AOS_Enabled)
	
		Pool.Clear();

		CHECK_EQUALS("Check destroyed state event occured.", TestObject->State, ENActorOperationalState::AOS_Destroyed)
	});
}

#endif //WITH_TESTS