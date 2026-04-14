// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NDynamicRef.h"
#include "NDynamicRefSubsystem.h"
#include "Developer/NTestUObject.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_CRITICAL(FNDynamicRefsTests_Subsystem_FastCollection_PreAllocated,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::FastCollection::PreAllocated",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies that Initialize() pre-allocates exactly NDR_Max buckets so every
    // enum value is a valid index with zero objects after subsystem creation.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        // Every bucket should start empty; none should crash on access.
        for (int32 i = 0; i < NDR_Max; ++i)
        {
            const ENDynamicRef Ref = static_cast<ENDynamicRef>(i);
            CHECK_EQUALS("Initial count should be zero for every ENDynamicRef bucket.",
                Subsystem->GetCount(Ref), 0)
        }
    });
}

N_TEST_CRITICAL(FNDynamicRefsTests_Subsystem_FastCollection_AddObject,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::FastCollection::AddObject",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies that AddObject increases the count and GetObjects returns the object.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObject(NDR_Player, Actor);

        CHECK_EQUALS("Count should be 1 after adding one object.", Subsystem->GetCount(NDR_Player), 1)

        TArray<UObject*> Objects = Subsystem->GetObjects(NDR_Player);
        CHECK_EQUALS("GetObjects should return exactly one entry.", Objects.Num(), 1)
        if (Objects.Num() > 0 && Objects[0] != Actor)
        {
            ADD_ERROR("GetObjects returned a different object than the one added.");
        }
    });
}

N_TEST_CRITICAL(FNDynamicRefsTests_Subsystem_FastCollection_AddObjectsDeduplication,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::FastCollection::AddObjectsDeduplication",
    N_TEST_CONTEXT_EDITOR)
{
    // FNDynamicRefCollection::Add uses AddUnique; adding the same object twice
    // should keep the count at 1.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObject(NDR_Enemy, Actor);
        Subsystem->AddObject(NDR_Enemy, Actor);

        CHECK_EQUALS("Duplicate add should not increase count beyond 1.", Subsystem->GetCount(NDR_Enemy), 1)
    });
}

N_TEST_CRITICAL(FNDynamicRefsTests_Subsystem_FastCollection_RemoveObject,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::FastCollection::RemoveObject",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies that RemoveObject decreases the count back to zero.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObject(NDR_NonPlayableCharacter, Actor);
        Subsystem->RemoveObject(NDR_NonPlayableCharacter, Actor);

        CHECK_EQUALS("Count should return to 0 after removing the only object.", Subsystem->GetCount(NDR_NonPlayableCharacter), 0)
    });
}

N_TEST_HIGH(FNDynamicRefsTests_Subsystem_FastCollection_AddObjectsArray,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::FastCollection::AddObjectsArray",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies that AddObjects (bulk) registers all supplied objects.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        constexpr int32 Count = 5;
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

        Subsystem->AddObjects(NDR_Target_A, Added);
        CHECK_EQUALS("Count should equal the number of distinct objects added.", Subsystem->GetCount(NDR_Target_A), Count)
    });
}

N_TEST_HIGH(FNDynamicRefsTests_Subsystem_FastCollection_RemoveObjectsArray,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::FastCollection::RemoveObjectsArray",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies that RemoveObjects (bulk) clears all supplied objects at once.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        constexpr int32 Count = 4;
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

        Subsystem->AddObjects(NDR_Spawn_A, Added);
        Subsystem->RemoveObjects(NDR_Spawn_A, Added);

        CHECK_EQUALS("Count should be 0 after bulk removal.", Subsystem->GetCount(NDR_Spawn_A), 0)
    });
}

N_TEST_CRITICAL(FNDynamicRefsTests_Subsystem_FastCollection_GetFirstObject,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::FastCollection::GetFirstObject",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies GetFirstObject returns the first added object and nullptr when empty.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        // Empty bucket should return nullptr safely.
        UObject* NullResult = Subsystem->GetFirstObject(NDR_Location_A);
        if (NullResult != nullptr)
        {
            ADD_ERROR("GetFirstObject should return nullptr for an empty bucket.");
        }

        AActor* First = World->SpawnActor<AActor>();
        AActor* Second = World->SpawnActor<AActor>();
        Subsystem->AddObject(NDR_Location_A, First);
        Subsystem->AddObject(NDR_Location_A, Second);

        UObject* Result = Subsystem->GetFirstObject(NDR_Location_A);
        if (Result != First)
        {
            ADD_ERROR("GetFirstObject did not return the first added object.");
        }
    });
}

N_TEST_CRITICAL(FNDynamicRefsTests_Subsystem_FastCollection_GetLastObject,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::FastCollection::GetLastObject",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies GetLastObject returns the last added object and nullptr when empty.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        UObject* NullResult = Subsystem->GetLastObject(NDR_Location_B);
        if (NullResult != nullptr)
        {
            ADD_ERROR("GetLastObject should return nullptr for an empty bucket.");
        }

        AActor* First = World->SpawnActor<AActor>();
        AActor* Last = World->SpawnActor<AActor>();
        Subsystem->AddObject(NDR_Location_B, First);
        Subsystem->AddObject(NDR_Location_B, Last);

        UObject* Result = Subsystem->GetLastObject(NDR_Location_B);
        if (Result != Last)
        {
            ADD_ERROR("GetLastObject did not return the last added object.");
        }
    });
}

N_TEST_HIGH(FNDynamicRefsTests_Subsystem_FastCollection_GetFirstActor,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::FastCollection::GetFirstActor",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies GetFirstActor skips non-AActor objects and returns the first actor.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        // Add a plain UObject first, then an Actor — GetFirstActor must skip the UObject.
        UObject* PlainObject = NewObject<UNTestUObject>(World);
        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObject(NDR_Objective_A, PlainObject);
        Subsystem->AddObject(NDR_Objective_A, Actor);

        AActor* Result = Subsystem->GetFirstActor(NDR_Objective_A);
        if (Result != Actor)
        {
            ADD_ERROR("GetFirstActor should skip non-AActor objects and return the spawned actor.");
        }
    });
}

N_TEST_HIGH(FNDynamicRefsTests_Subsystem_FastCollection_GetLastActor,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::FastCollection::GetLastActor",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies GetLastActor iterates in reverse and returns the last actor.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        AActor* First = World->SpawnActor<AActor>();
        AActor* Last = World->SpawnActor<AActor>();
        Subsystem->AddObject(NDR_Objective_B, First);
        Subsystem->AddObject(NDR_Objective_B, Last);

        AActor* Result = Subsystem->GetLastActor(NDR_Objective_B);
        if (Result != Last)
        {
            ADD_ERROR("GetLastActor should return the last added actor.");
        }
    });
}

N_TEST_HIGH(FNDynamicRefsTests_Subsystem_FastCollection_GetActors_FiltersNonActors,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::FastCollection::GetActors::FiltersNonActors",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies GetActors only returns AActor objects, silently dropping plain UObjects.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        UObject* Plain = NewObject<UNTestUObject>(World);
        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObject(NDR_Secret_A, Plain);
        Subsystem->AddObject(NDR_Secret_A, Actor);

        TArray<AActor*> Actors = Subsystem->GetActors(NDR_Secret_A);
        CHECK_EQUALS("GetActors should return only 1 actor (non-actors filtered).", Actors.Num(), 1)
        if (Actors.Num() > 0 && Actors[0] != Actor)
        {
            ADD_ERROR("GetActors returned an unexpected actor pointer.");
        }
    });
}

N_TEST_HIGH(FNDynamicRefsTests_Subsystem_FastCollection_GetDynamicRefs,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::FastCollection::GetDynamicRefs",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies GetDynamicRefs only lists non-empty buckets.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        const int32 CountBefore = Subsystem->GetDynamicRefs().Num();

        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObject(NDR_Pickup_A, Actor);

        const TArray<ENDynamicRef> ActiveRefs = Subsystem->GetDynamicRefs();
        CHECK_EQUALS("GetDynamicRefs should list exactly one more bucket after adding to NDR_Pickup_A.",
            ActiveRefs.Num(), CountBefore + 1)

        if (!ActiveRefs.Contains(NDR_Pickup_A))
        {
            ADD_ERROR("GetDynamicRefs does not contain NDR_Pickup_A despite it having objects.");
        }
    });
}

N_TEST_CRITICAL(FNDynamicRefsTests_Subsystem_NamedCollection_AddObjectByName,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::NamedCollection::AddObjectByName",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies AddObjectByName lazily creates a bucket and registers the object.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        const FName TestName = TEXT("TestPlayerRef");
        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TestName, Actor);

        CHECK_EQUALS("Count by name should be 1 after adding one object.", Subsystem->GetCountByName(TestName), 1)

        TArray<UObject*> Objects = Subsystem->GetObjectsByName(TestName);
        CHECK_EQUALS("GetObjectsByName should return exactly one entry.", Objects.Num(), 1)
        if (Objects.Num() > 0 && Objects[0] != Actor)
        {
            ADD_ERROR("GetObjectsByName returned a different object than the one added.");
        }
    });
}

N_TEST_CRITICAL(FNDynamicRefsTests_Subsystem_NamedCollection_RemoveObjectByName,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::NamedCollection::RemoveObjectByName",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies RemoveObjectByName decreases the count to zero.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        const FName TestName = TEXT("TestEnemyRef");
        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TestName, Actor);
        Subsystem->RemoveObjectByName(TestName, Actor);

        CHECK_EQUALS("Count should be 0 after removing the only named object.", Subsystem->GetCountByName(TestName), 0)
    });
}

N_TEST_HIGH(FNDynamicRefsTests_Subsystem_NamedCollection_RemoveNonExistentBucket,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::NamedCollection::RemoveNonExistentBucket",
    N_TEST_CONTEXT_EDITOR)
{
    // RemoveObjectByName on a name that was never added should not crash.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        AActor* Actor = World->SpawnActor<AActor>();
        // No add — remove should be a no-op.
        Subsystem->RemoveObjectByName(TEXT("NonExistentBucket"), Actor);

        CHECK_EQUALS("Count for a never-added name should still be 0.", Subsystem->GetCountByName(TEXT("NonExistentBucket")), 0)
    });
}

N_TEST_HIGH(FNDynamicRefsTests_Subsystem_NamedCollection_AddObjectsByName,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::NamedCollection::AddObjectsByName",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies AddObjectsByName bulk-registers all supplied objects under a name.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        const FName TestName = TEXT("TestBulkName");
        constexpr int32 Count = 3;
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

        Subsystem->AddObjectsByName(TestName, Added);
        CHECK_EQUALS("Count should equal the number of objects added by name.", Subsystem->GetCountByName(TestName), Count)
    });
}

N_TEST_HIGH(FNDynamicRefsTests_Subsystem_NamedCollection_GetFirstObjectByName,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::NamedCollection::GetFirstObjectByName",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies GetFirstObjectByName returns the first added object; nullptr for unknown names.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        UObject* NullResult = Subsystem->GetFirstObjectByName(TEXT("UnknownName"));
        if (NullResult != nullptr)
        {
            ADD_ERROR("GetFirstObjectByName should return nullptr for an unknown name.");
        }

        const FName TestName = TEXT("TestFirstByName");
        AActor* First = World->SpawnActor<AActor>();
        AActor* Second = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TestName, First);
        Subsystem->AddObjectByName(TestName, Second);

        UObject* Result = Subsystem->GetFirstObjectByName(TestName);
        if (Result != First)
        {
            ADD_ERROR("GetFirstObjectByName did not return the first added object.");
        }
    });
}

N_TEST_HIGH(FNDynamicRefsTests_Subsystem_NamedCollection_GetLastObjectByName,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::NamedCollection::GetLastObjectByName",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies GetLastObjectByName returns the last added object; nullptr for unknown names.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        UObject* NullResult = Subsystem->GetLastObjectByName(TEXT("UnknownName2"));
        if (NullResult != nullptr)
        {
            ADD_ERROR("GetLastObjectByName should return nullptr for an unknown name.");
        }

        const FName TestName = TEXT("TestLastByName");
        AActor* First = World->SpawnActor<AActor>();
        AActor* Last = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TestName, First);
        Subsystem->AddObjectByName(TestName, Last);

        UObject* Result = Subsystem->GetLastObjectByName(TestName);
        if (Result != Last)
        {
            ADD_ERROR("GetLastObjectByName did not return the last added object.");
        }
    });
}

N_TEST_HIGH(FNDynamicRefsTests_Subsystem_NamedCollection_GetActorsByName,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::NamedCollection::GetActorsByName",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies GetActorsByName filters out non-AActor objects.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        const FName TestName = TEXT("TestActorsByName");
        UObject* Plain = NewObject<UNTestUObject>(World);
        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TestName, Plain);
        Subsystem->AddObjectByName(TestName, Actor);

        TArray<AActor*> Actors = Subsystem->GetActorsByName(TestName);
        CHECK_EQUALS("GetActorsByName should return only 1 actor (non-actors filtered).", Actors.Num(), 1)
    });
}

N_TEST_HIGH(FNDynamicRefsTests_Subsystem_NamedCollection_GetNames,
    "NEXUS::UnitTests::NDynamicRefs::Subsystem::NamedCollection::GetNames",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies GetNames includes any name that has been added.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem)
        {
            ADD_ERROR("Could not retrieve UNDynamicRefSubsystem from editor world.");
            return;
        }

        const FName TestName = TEXT("TestGetNames_Unique");
        const int32 CountBefore = Subsystem->GetNames().Num();

        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TestName, Actor);

        const TArray<FName> Names = Subsystem->GetNames();
        CHECK_EQUALS("GetNames should contain one more entry after adding a new named ref.",
            Names.Num(), CountBefore + 1)
        if (!Names.Contains(TestName))
        {
            ADD_ERROR("GetNames does not contain the newly added name.");
        }
    });
}

#endif //WITH_TESTS
