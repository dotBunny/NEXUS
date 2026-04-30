// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NDynamicRef.h"
#include "NDynamicRefSubsystem.h"
#include "Developer/NTestObject.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_CRITICAL(UNDynamicRefSubsystemTests_Delegates_OnAdded,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::Delegates::OnAdded",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies that AddObject fires the OnAdded delegate exactly once per call
    // with the correct ENDynamicRef and UObject pointer.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        const TUniquePtr<FNTestObject> TestObject = MakeUnique<FNTestObject>();
        FNTestObject* Tracker = TestObject.Get();
        ENDynamicRef ReceivedRef = NDR_None;
        UObject* ReceivedObject = nullptr;

        FDelegateHandle Handle = Subsystem->OnAdded.AddLambda(
            [Tracker, &ReceivedRef, &ReceivedObject](const ENDynamicRef Ref, UObject* Object)
            {
                Tracker->Counter++;
                ReceivedRef = Ref;
                ReceivedObject = Object;
            });

        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObject(NDR_Player, Actor);

        CHECK_EQUALS("OnAdded should fire exactly once.", Tracker->Counter, 1)
        if (ReceivedRef != NDR_Player)
        {
            ADD_ERROR("OnAdded received the wrong ENDynamicRef value.");
        }
        if (ReceivedObject != Actor)
        {
            ADD_ERROR("OnAdded received the wrong UObject pointer.");
        }
    });
}

N_TEST_CRITICAL(UNDynamicRefSubsystemTests_Delegates_OnRemoved,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::Delegates::OnRemoved",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies that RemoveObject fires the OnRemoved delegate exactly once per call
    // with the correct ENDynamicRef and UObject pointer.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        const TUniquePtr<FNTestObject> TestObject = MakeUnique<FNTestObject>();
        FNTestObject* Tracker = TestObject.Get();
        ENDynamicRef ReceivedRef = NDR_None;
        UObject* ReceivedObject = nullptr;

        FDelegateHandle Handle = Subsystem->OnRemoved.AddLambda(
            [Tracker, &ReceivedRef, &ReceivedObject](const ENDynamicRef Ref, UObject* Object)
            {
                Tracker->Counter++;
                ReceivedRef = Ref;
                ReceivedObject = Object;
            });

        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObject(NDR_Enemy, Actor);
        Subsystem->RemoveObject(NDR_Enemy, Actor);

        CHECK_EQUALS("OnRemoved should fire exactly once.", Tracker->Counter, 1)
        if (ReceivedRef != NDR_Enemy)
        {
            ADD_ERROR("OnRemoved received the wrong ENDynamicRef value.");
        }
        if (ReceivedObject != Actor)
        {
            ADD_ERROR("OnRemoved received the wrong UObject pointer.");
        }
    });
}

N_TEST_HIGH(UNDynamicRefSubsystemTests_Delegates_OnAdded_BulkFiresPerObject,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::Delegates::OnAdded::BulkFiresPerObject",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies AddObjects fires OnAdded once per object in the supplied array.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        constexpr int32 Count = 4;
        const TUniquePtr<FNTestObject> TestObject = MakeUnique<FNTestObject>();
        FNTestObject* Tracker = TestObject.Get();

        FDelegateHandle Handle = Subsystem->OnAdded.AddLambda(
            [Tracker](ENDynamicRef, UObject*) { Tracker->Counter++; });

        TArray<UObject*> Added;
        Added.Reserve(Count);
        TArray<AActor*> Spawned;
        Spawned.Reserve(Count);
        for (int32 i = 0; i < Count; ++i)
        {
            AActor* A = World->SpawnActor<AActor>();
            Spawned.Add(A);
            Added.Add(A);
        }

        Subsystem->AddObjects(NDR_Item_A, Added);
        CHECK_EQUALS("OnAdded should fire once per object in the bulk add.", Tracker->Counter, Count)
    });
}

N_TEST_HIGH(UNDynamicRefSubsystemTests_Delegates_OnRemoved_DoesNotFireForMissingObject,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::Delegates::OnRemoved::DoesNotFireForMissingObject",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies RemoveObject still fires OnRemoved even when the object was never
    // registered (subsystem does not guard the broadcast).
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        const TUniquePtr<FNTestObject> TestObject = MakeUnique<FNTestObject>();
        FNTestObject* Tracker = TestObject.Get();

        FDelegateHandle Handle = Subsystem->OnRemoved.AddLambda(
            [Tracker](ENDynamicRef, UObject*) { Tracker->Counter++; });

        AActor* Actor = World->SpawnActor<AActor>();
        // Intentionally skip AddObject to test remove-without-add.
        Subsystem->RemoveObject(NDR_NonPlayableCharacter, Actor);
    	
        CHECK_EQUALS("OnRemoved fired even for an object that was never added.", Tracker->Counter, 0)
    });
}

N_TEST_CRITICAL(UNDynamicRefSubsystemTests_Delegates_OnAddedByName,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::Delegates::OnAddedByName",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies that AddObjectByName fires the OnAddedByName delegate with the
    // correct FName and UObject pointer.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        const TUniquePtr<FNTestObject> TestObject = MakeUnique<FNTestObject>();
        FNTestObject* Tracker = TestObject.Get();
        FName ReceivedName = NAME_None;
        UObject* ReceivedObject = nullptr;

        FDelegateHandle Handle = Subsystem->OnAddedByName.AddLambda(
            [Tracker, &ReceivedName, &ReceivedObject](const FName Name, UObject* Object)
            {
                Tracker->Counter++;
                ReceivedName = Name;
                ReceivedObject = Object;
            });

        const FName TestName = TEXT("TestDelegateAdd");
        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TestName, Actor);

        CHECK_EQUALS("OnAddedByName should fire exactly once.", Tracker->Counter, 1)
        if (ReceivedName != TestName)
        {
            ADD_ERROR("OnAddedByName received the wrong FName value.");
        }
        if (ReceivedObject != Actor)
        {
            ADD_ERROR("OnAddedByName received the wrong UObject pointer.");
        }
    });
}

N_TEST_CRITICAL(UNDynamicRefSubsystemTests_Delegates_OnRemovedByName,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::Delegates::OnRemovedByName",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies that RemoveObjectByName fires the OnRemovedByName delegate with
    // the correct FName and UObject pointer.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        const TUniquePtr<FNTestObject> TestObject = MakeUnique<FNTestObject>();
        FNTestObject* Tracker = TestObject.Get();
        FName ReceivedName = NAME_None;
        UObject* ReceivedObject = nullptr;

        FDelegateHandle Handle = Subsystem->OnRemovedByName.AddLambda(
            [Tracker, &ReceivedName, &ReceivedObject](const FName Name, UObject* Object)
            {
                Tracker->Counter++;
                ReceivedName = Name;
                ReceivedObject = Object;
            });

        const FName TestName = TEXT("TestDelegateRemove");
        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TestName, Actor);
        Subsystem->RemoveObjectByName(TestName, Actor);

        CHECK_EQUALS("OnRemovedByName should fire exactly once.", Tracker->Counter, 1)
        if (ReceivedName != TestName)
        {
            ADD_ERROR("OnRemovedByName received the wrong FName value.");
        }
        if (ReceivedObject != Actor)
        {
            ADD_ERROR("OnRemovedByName received the wrong UObject pointer.");
        }
    });
}

N_TEST_HIGH(UNDynamicRefSubsystemTests_Delegates_OnRemovedByName_NotFiredForUnknownBucket,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::Delegates::OnRemovedByName::NotFiredForUnknownBucket",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies that RemoveObjectByName does NOT fire OnRemovedByName when the
    // named bucket does not exist.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        const TUniquePtr<FNTestObject> TestObject = MakeUnique<FNTestObject>();
        FNTestObject* Tracker = TestObject.Get();

        FDelegateHandle Handle = Subsystem->OnRemovedByName.AddLambda(
            [Tracker](FName, UObject*) { Tracker->Counter++; });

        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->RemoveObjectByName(TEXT("NeverAddedBucket"), Actor);

        CHECK_EQUALS("OnRemovedByName must not fire for a bucket that does not exist.", Tracker->Counter, 0)
    });
}

#endif //WITH_TESTS
