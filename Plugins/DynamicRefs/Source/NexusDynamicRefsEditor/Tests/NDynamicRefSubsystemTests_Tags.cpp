// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "NDynamicRefComponent.h"
#include "NDynamicRefSubsystem.h"
#include "Developer/NTestUObject.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

// All tests use the editor-only tags A/B/C defined in NDynamicRefsGameplayTags.cpp,
// resolved by name via the GameplayTagsManager. The runtime module's
// UE_DEFINE_GAMEPLAY_TAG entries register these tags before any test runs, so
// there are no late-registration ensures. Each test gets a fresh PIE world (and
// therefore a fresh subsystem), so tag buckets cannot leak between tests.

namespace NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness
{
    static FGameplayTag EditorTagA()
    {
        static const FGameplayTag Tag = UGameplayTagsManager::Get().RequestGameplayTag(FName(TEXT("NEXUS.DynamicRefs.EditorOnly.A")));
        return Tag;
    }
    static FGameplayTag EditorTagB()
    {
        static const FGameplayTag Tag = UGameplayTagsManager::Get().RequestGameplayTag(FName(TEXT("NEXUS.DynamicRefs.EditorOnly.B")));
        return Tag;
    }

    static void SetComponentTagReferences(UNDynamicRefComponent* Component, const FGameplayTagContainer& Tags)
    {
        // TagReferences is protected; reach it via reflection so the test does not
        // require changing the production API surface.
        FProperty* Prop = UNDynamicRefComponent::StaticClass()->FindPropertyByName(TEXT("TagReferences"));
        if (Prop == nullptr) return;
        FGameplayTagContainer* Container = Prop->ContainerPtrToValuePtr<FGameplayTagContainer>(Component);
        if (Container == nullptr) return;
        *Container = Tags;
    }
}

N_TEST_CRITICAL(UNDynamicRefSubsystemTests_TagCollection_AddObjectByTag,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::TagCollection::AddObjectByTag",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies that an object added via the FName route under Tag.GetTagName() is
    // returned by GetObjectsByTag (storage is shared between named and tag references).
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNDynamicRefSubsystem, World)

        const FGameplayTag TagA = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagA();
        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TagA.GetTagName(), Actor);

        CHECK_EQUALS("GetCountByTag should report 1 after a single add.", Subsystem->GetCountByTag(TagA), 1)

        TArray<UObject*> Objects = Subsystem->GetObjectsByTag(TagA);
        CHECK_EQUALS("GetObjectsByTag should return exactly one entry.", Objects.Num(), 1)
        if (Objects.Num() > 0 && Objects[0] != Actor)
        {
            ADD_ERROR("GetObjectsByTag returned a different object than the one added.");
        }
    });
}

N_TEST_HIGH(UNDynamicRefSubsystemTests_TagCollection_EmptyTag,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::TagCollection::EmptyTag",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies that querying with FGameplayTag::EmptyTag short-circuits to empty results.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNDynamicRefSubsystem, World)

        CHECK_EQUALS("GetCountByTag(Empty) should be 0.", Subsystem->GetCountByTag(FGameplayTag::EmptyTag), 0)
        CHECK_EQUALS("GetObjectsByTag(Empty) should return empty array.", Subsystem->GetObjectsByTag(FGameplayTag::EmptyTag).Num(), 0)
        CHECK_EQUALS("GetActorsByTag(Empty) should return empty array.", Subsystem->GetActorsByTag(FGameplayTag::EmptyTag).Num(), 0)
        if (Subsystem->GetFirstObjectByTag(FGameplayTag::EmptyTag) != nullptr)
        {
            ADD_ERROR("GetFirstObjectByTag(Empty) should return nullptr.");
        }
        if (Subsystem->GetLastObjectByTag(FGameplayTag::EmptyTag) != nullptr)
        {
            ADD_ERROR("GetLastObjectByTag(Empty) should return nullptr.");
        }
    });
}

N_TEST_HIGH(UNDynamicRefSubsystemTests_TagCollection_UnknownTag,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::TagCollection::UnknownTag",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies that a registered tag that was never added to any bucket returns empty / nullptr.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNDynamicRefSubsystem, World)

        const FGameplayTag UnusedTag = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagA();

        CHECK_EQUALS("GetCountByTag for an unused tag should be 0.", Subsystem->GetCountByTag(UnusedTag), 0)
        CHECK_EQUALS("GetObjectsByTag for an unused tag should be empty.", Subsystem->GetObjectsByTag(UnusedTag).Num(), 0)
        if (Subsystem->GetFirstObjectByTag(UnusedTag) != nullptr)
        {
            ADD_ERROR("GetFirstObjectByTag for an unused tag should return nullptr.");
        }
    });
}

N_TEST_HIGH(UNDynamicRefSubsystemTests_TagCollection_ParityWithByName,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::TagCollection::ParityWithByName",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies tag-flavored queries return the same data as the FName equivalents
    // when the FName matches Tag.GetTagName().
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNDynamicRefSubsystem, World)

        const FGameplayTag TagA = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagA();
        AActor* A = World->SpawnActor<AActor>();
        AActor* B = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TagA.GetTagName(), A);
        Subsystem->AddObjectByName(TagA.GetTagName(), B);

        CHECK_EQUALS("GetCountByTag matches GetCountByName.",
            Subsystem->GetCountByTag(TagA), Subsystem->GetCountByName(TagA.GetTagName()))
        CHECK_EQUALS("GetObjectsByTag count matches GetObjectsByName count.",
            Subsystem->GetObjectsByTag(TagA).Num(), Subsystem->GetObjectsByName(TagA.GetTagName()).Num())
        CHECK_EQUALS("GetActorsByTag count matches GetActorsByName count.",
            Subsystem->GetActorsByTag(TagA).Num(), Subsystem->GetActorsByName(TagA.GetTagName()).Num())
    });
}

N_TEST_HIGH(UNDynamicRefSubsystemTests_TagCollection_FirstLastByTag,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::TagCollection::FirstLastByTag",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies GetFirstObjectByTag / GetLastObjectByTag honor registration order.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNDynamicRefSubsystem, World)

        const FGameplayTag TagA = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagA();
        AActor* First = World->SpawnActor<AActor>();
        AActor* Last = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TagA.GetTagName(), First);
        Subsystem->AddObjectByName(TagA.GetTagName(), Last);

        if (Subsystem->GetFirstObjectByTag(TagA) != First)
        {
            ADD_ERROR("GetFirstObjectByTag did not return the first added object.");
        }
        if (Subsystem->GetLastObjectByTag(TagA) != Last)
        {
            ADD_ERROR("GetLastObjectByTag did not return the last added object.");
        }
        if (Subsystem->GetFirstActorByTag(TagA) != First)
        {
            ADD_ERROR("GetFirstActorByTag did not return the first added actor.");
        }
        if (Subsystem->GetLastActorByTag(TagA) != Last)
        {
            ADD_ERROR("GetLastActorByTag did not return the last added actor.");
        }
    });
}

N_TEST_CRITICAL(UNDynamicRefSubsystemTests_TagCollection_GetObjectsByAnyTags_Union,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::TagCollection::GetObjectsByAnyTags::Union",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies union semantics across multiple tags with shared and unique members; result is deduplicated.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNDynamicRefSubsystem, World)

        const FGameplayTag TagA = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagA();
        const FGameplayTag TagB = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagB();

        AActor* OnlyA = World->SpawnActor<AActor>();
        AActor* OnlyB = World->SpawnActor<AActor>();
        AActor* Both = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TagA.GetTagName(), OnlyA);
        Subsystem->AddObjectByName(TagB.GetTagName(), OnlyB);
        Subsystem->AddObjectByName(TagA.GetTagName(), Both);
        Subsystem->AddObjectByName(TagB.GetTagName(), Both);

        FGameplayTagContainer Query;
        Query.AddTag(TagA);
        Query.AddTag(TagB);

        TArray<UObject*> Result = Subsystem->GetObjectsByAnyTags(Query);
        CHECK_EQUALS("AnyTags union should return 3 unique objects.", Result.Num(), 3)
        if (!Result.Contains(OnlyA)) ADD_ERROR("AnyTags result missing OnlyA.");
        if (!Result.Contains(OnlyB)) ADD_ERROR("AnyTags result missing OnlyB.");
        if (!Result.Contains(Both)) ADD_ERROR("AnyTags result missing Both.");
        CHECK_EQUALS("GetCountByAnyTags should match union size.", Subsystem->GetCountByAnyTags(Query), 3)
    });
}

N_TEST_HIGH(UNDynamicRefSubsystemTests_TagCollection_GetObjectsByAnyTags_PartiallyEmpty,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::TagCollection::GetObjectsByAnyTags::PartiallyEmpty",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies AnyTags still returns the union when some requested tags have no bucket.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNDynamicRefSubsystem, World)

        const FGameplayTag TagA = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagA();
        const FGameplayTag TagB = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagB();

        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TagA.GetTagName(), Actor);
        // TagB has no registrations.

        FGameplayTagContainer Query;
        Query.AddTag(TagA);
        Query.AddTag(TagB);

        TArray<UObject*> Result = Subsystem->GetObjectsByAnyTags(Query);
        CHECK_EQUALS("AnyTags with one empty bucket should still return populated tag's objects.", Result.Num(), 1)
        if (Result.Num() > 0 && Result[0] != Actor)
        {
            ADD_ERROR("AnyTags returned an unexpected object.");
        }
    });
}

N_TEST_CRITICAL(UNDynamicRefSubsystemTests_TagCollection_GetObjectsByAllTags_Intersection,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::TagCollection::GetObjectsByAllTags::Intersection",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies AllTags returns only objects present under every requested tag.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNDynamicRefSubsystem, World)

        const FGameplayTag TagA = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagA();
        const FGameplayTag TagB = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagB();

        AActor* OnlyA = World->SpawnActor<AActor>();
        AActor* OnlyB = World->SpawnActor<AActor>();
        AActor* Both = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TagA.GetTagName(), OnlyA);
        Subsystem->AddObjectByName(TagB.GetTagName(), OnlyB);
        Subsystem->AddObjectByName(TagA.GetTagName(), Both);
        Subsystem->AddObjectByName(TagB.GetTagName(), Both);

        FGameplayTagContainer Query;
        Query.AddTag(TagA);
        Query.AddTag(TagB);

        TArray<UObject*> Result = Subsystem->GetObjectsByAllTags(Query);
        CHECK_EQUALS("AllTags intersection should return exactly 1 object.", Result.Num(), 1)
        if (Result.Num() > 0 && Result[0] != Both)
        {
            ADD_ERROR("AllTags intersection should be the object registered under both tags.");
        }
        CHECK_EQUALS("GetCountByAllTags should match intersection size.", Subsystem->GetCountByAllTags(Query), 1)
    });
}

N_TEST_HIGH(UNDynamicRefSubsystemTests_TagCollection_GetObjectsByAllTags_MissingTag,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::TagCollection::GetObjectsByAllTags::MissingTag",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies AllTags returns empty when any requested tag has no bucket.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNDynamicRefSubsystem, World)

        const FGameplayTag TagA = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagA();
        const FGameplayTag TagB = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagB();

        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TagA.GetTagName(), Actor);

        FGameplayTagContainer Query;
        Query.AddTag(TagA);
        Query.AddTag(TagB);

        CHECK_EQUALS("AllTags should be empty when one tag has no bucket.", Subsystem->GetObjectsByAllTags(Query).Num(), 0)
    });
}

N_TEST_MEDIUM(UNDynamicRefSubsystemTests_TagCollection_GetObjectsByAllTags_SingleTag,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::TagCollection::GetObjectsByAllTags::SingleTag",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies AllTags with a single-tag container equals GetObjectsByTag for that tag.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNDynamicRefSubsystem, World)

        const FGameplayTag TagA = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagA();
        AActor* A = World->SpawnActor<AActor>();
        AActor* B = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TagA.GetTagName(), A);
        Subsystem->AddObjectByName(TagA.GetTagName(), B);

        FGameplayTagContainer Query;
        Query.AddTag(TagA);

        CHECK_EQUALS("AllTags(SingleTag) should equal GetObjectsByTag count.",
            Subsystem->GetObjectsByAllTags(Query).Num(), Subsystem->GetObjectsByTag(TagA).Num())
    });
}

N_TEST_HIGH(UNDynamicRefSubsystemTests_TagCollection_GetActorsByAnyTags_FiltersNonActors,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::TagCollection::GetActorsByAnyTags::FiltersNonActors",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies GetActorsByAnyTags / GetActorsByAllTags drop non-AActor UObjects.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNDynamicRefSubsystem, World)

        const FGameplayTag TagA = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagA();
        const FGameplayTag TagB = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagB();

        UObject* Plain = NewObject<UNTestUObject>(World);
        AActor* Actor = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TagA.GetTagName(), Plain);
        Subsystem->AddObjectByName(TagA.GetTagName(), Actor);
        Subsystem->AddObjectByName(TagB.GetTagName(), Actor);

        FGameplayTagContainer Any;
        Any.AddTag(TagA);
        Any.AddTag(TagB);
        TArray<AActor*> AnyActors = Subsystem->GetActorsByAnyTags(Any);
        CHECK_EQUALS("AnyTags actor filter should return 1 actor.", AnyActors.Num(), 1)

        FGameplayTagContainer All;
        All.AddTag(TagA);
        All.AddTag(TagB);
        TArray<AActor*> AllActors = Subsystem->GetActorsByAllTags(All);
        CHECK_EQUALS("AllTags actor filter should return 1 actor.", AllActors.Num(), 1)
    });
}

N_TEST_HIGH(UNDynamicRefSubsystemTests_TagCollection_GetTags,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::TagCollection::GetTags",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies GetTags reports buckets whose FName resolves to a registered FGameplayTag,
    // skipping FName-only buckets.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNDynamicRefSubsystem, World)

        const FGameplayTag TagA = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagA();
        const FGameplayTag TagB = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagB();
        const FName RawName = TEXT("Nexus_Tests_DynamicRefs_GetTags_RawName_NotATag");

        AActor* A = World->SpawnActor<AActor>();
        AActor* B = World->SpawnActor<AActor>();
        AActor* C = World->SpawnActor<AActor>();
        Subsystem->AddObjectByName(TagA.GetTagName(), A);
        Subsystem->AddObjectByName(TagB.GetTagName(), B);
        Subsystem->AddObjectByName(RawName, C);

        TArray<FGameplayTag> Tags = Subsystem->GetTags();
        if (!Tags.Contains(TagA)) ADD_ERROR("GetTags should contain TagA.");
        if (!Tags.Contains(TagB)) ADD_ERROR("GetTags should contain TagB.");
        for (const FGameplayTag& Tag : Tags)
        {
            if (Tag.GetTagName() == RawName)
            {
                ADD_ERROR("GetTags should not contain the raw FName bucket.");
            }
        }
    });
}

N_TEST_CRITICAL(UNDynamicRefSubsystemTests_TagCollection_Component_BeginPlayEndPlay,
    "NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystem::TagCollection::Component::BeginPlayEndPlay",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies an actor with a UNDynamicRefComponent whose TagReferences contains two
    // tags is registered on BeginPlay and unregistered on EndPlay/destroy.
    FNTestUtils::WorldTestChecked(EWorldType::PIE, [this](UWorld* World)
    {
        N_TEST_GET_SUBSYSTEM_CHECKED(Subsystem, UNDynamicRefSubsystem, World)

        const FGameplayTag TagA = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagA();
        const FGameplayTag TagB = NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagB();

        AActor* Actor = World->SpawnActorDeferred<AActor>(AActor::StaticClass(), FTransform::Identity);
        if (Actor == nullptr)
        {
            ADD_ERROR("SpawnActorDeferred returned nullptr.");
            return;
        }

        UNDynamicRefComponent* Component = NewObject<UNDynamicRefComponent>(Actor);
        FGameplayTagContainer TagContainer;
        TagContainer.AddTag(TagA);
        TagContainer.AddTag(TagB);
        NEXUS::UnitTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::SetComponentTagReferences(Component, TagContainer);
        Component->RegisterComponent();

        Actor->FinishSpawning(FTransform::Identity);

        CHECK_EQUALS("Actor should be registered under TagA after BeginPlay.", Subsystem->GetCountByTag(TagA), 1)
        CHECK_EQUALS("Actor should be registered under TagB after BeginPlay.", Subsystem->GetCountByTag(TagB), 1)
        if (Subsystem->GetFirstActorByTag(TagA) != Actor)
        {
            ADD_ERROR("GetFirstActorByTag(TagA) did not return the spawned actor.");
        }

        World->DestroyActor(Actor);

        CHECK_EQUALS("Actor should be unregistered from TagA after EndPlay.", Subsystem->GetCountByTag(TagA), 0)
        CHECK_EQUALS("Actor should be unregistered from TagB after EndPlay.", Subsystem->GetCountByTag(TagB), 0)
    });
}

#endif //WITH_TESTS
