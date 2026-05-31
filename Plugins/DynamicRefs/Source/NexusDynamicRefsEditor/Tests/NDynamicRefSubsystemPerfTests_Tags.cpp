// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "Misc/Timespan.h"
#include "NDynamicRefSubsystem.h"
#include "Developer/NTestUtils.h"
#include "Macros/NTestMacros.h"

// Uses the editor-only tags A/B/C defined in NDynamicRefsGameplayTags.cpp, resolved
// by name via the GameplayTagsManager (the FNativeGameplayTag symbols are not
// DLL-exported, but the tag data is registered during runtime module load).
namespace NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness
{
    constexpr int32 ObjectCount = 1000;
    constexpr int32 QueryCount = 100;
    constexpr int32 ObjectsPerTag = 250;
    constexpr int32 SharedObjectCount = 50;

    constexpr float GetByTagMaxDuration = 0.15f;
    constexpr float GetByAnyTagsMaxDuration = 12.0f;
    constexpr float GetByAllTagsMaxDuration = 5.0f;
    constexpr float GetTagsMaxDuration = 5.0f;

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
    static FGameplayTag EditorTagC()
    {
        static const FGameplayTag Tag = UGameplayTagsManager::Get().RequestGameplayTag(FName(TEXT("NEXUS.DynamicRefs.EditorOnly.C")));
        return Tag;
    }
}

class FNDynamicRefSubsystemPerfTests_Tags
{
public:
    // Measures the per-call cost of GetObjectsByTag (single FName wrapper) against a 1000-object bucket.
    static void GetObjectsByTag(UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem) return;

        const FGameplayTag Tag = NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagA();
        for (int32 i = 0; i < NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::ObjectCount; ++i)
        {
            Subsystem->AddObjectByName(Tag.GetTagName(), World->SpawnActor<AActor>());
        }

        // TEST
        {
            N_TEST_TIMER_SCOPE(UNDynamicRefSubsystemPerfTests_Tags_GetObjectsByTag,
                NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::GetByTagMaxDuration)
            for (int32 i = 0; i < NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::QueryCount; ++i)
            {
                Subsystem->GetObjectsByTag(Tag);
            }
            NTestTimer.ManualStop();
        }
    }

    // Measures GetObjectsByAnyTags across the three editor-only tags, each holding ObjectsPerTag unique entries.
    static void GetObjectsByAnyTags(UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem) return;

        const FGameplayTag Tags[] = {
            NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagA(),
            NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagB(),
            NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagC(),
        };
        FGameplayTagContainer Container;
        for (const FGameplayTag& Tag : Tags)
        {
            Container.AddTag(Tag);
            for (int32 i = 0; i < NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::ObjectsPerTag; ++i)
            {
                Subsystem->AddObjectByName(Tag.GetTagName(), World->SpawnActor<AActor>());
            }
        }

        // TEST
        {
            N_TEST_TIMER_SCOPE(UNDynamicRefSubsystemPerfTests_Tags_GetObjectsByAnyTags,
                NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::GetByAnyTagsMaxDuration)
            for (int32 i = 0; i < NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::QueryCount; ++i)
            {
                Subsystem->GetObjectsByAnyTags(Container);
            }
            NTestTimer.ManualStop();
        }
    }

    // Measures GetObjectsByAllTags across the three editor-only tags with SharedObjectCount shared members.
    static void GetObjectsByAllTags(UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem) return;

        // Pre-spawn the shared actors (registered under every tag) outside the timed region.
        TArray<AActor*> Shared;
        Shared.Reserve(NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::SharedObjectCount);
        for (int32 i = 0; i < NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::SharedObjectCount; ++i)
        {
            Shared.Add(World->SpawnActor<AActor>());
        }

        const FGameplayTag Tags[] = {
            NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagA(),
            NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagB(),
            NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::EditorTagC(),
        };
        FGameplayTagContainer Container;
        for (const FGameplayTag& Tag : Tags)
        {
            Container.AddTag(Tag);

            // Unique-to-this-tag members.
            const int32 Unique = NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::ObjectsPerTag
                - NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::SharedObjectCount;
            for (int32 i = 0; i < Unique; ++i)
            {
                Subsystem->AddObjectByName(Tag.GetTagName(), World->SpawnActor<AActor>());
            }
            // Shared members.
            for (AActor* Actor : Shared)
            {
                Subsystem->AddObjectByName(Tag.GetTagName(), Actor);
            }
        }

        // TEST
        {
            N_TEST_TIMER_SCOPE(UNDynamicRefSubsystemPerfTests_Tags_GetObjectsByAllTags,
                NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::GetByAllTagsMaxDuration)
            for (int32 i = 0; i < NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::QueryCount; ++i)
            {
                Subsystem->GetObjectsByAllTags(Container);
            }
            NTestTimer.ManualStop();
        }
    }

    // Measures GetTags against a populated map of FName buckets. Each bucket triggers
    // a RequestGameplayTag resolve — the cost we're measuring — regardless of whether
    // the FName matches a registered tag.
    static void GetTags(UWorld* World)
    {
        UNDynamicRefSubsystem* Subsystem = UNDynamicRefSubsystem::Get(World);
        if (!Subsystem) return;

        AActor* Anchor = World->SpawnActor<AActor>();
        for (int32 i = 0; i < NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::ObjectCount; ++i)
        {
            Subsystem->AddObjectByName(
                FName(*FString::Printf(TEXT("Nexus.Tests.PerfTests.DynamicRefs.GetTags.T%d"), i)),
                Anchor);
        }

        // TEST
        {
            N_TEST_TIMER_SCOPE(UNDynamicRefSubsystemPerfTests_Tags_GetTags,
                NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystemTagsHarness::GetTagsMaxDuration)
            Subsystem->GetTags();
            NTestTimer.ManualStop();
        }
    }
};

N_TEST_PERF(UNDynamicRefSubsystemPerfTests_Tags_GetObjectsByTag,
    "NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystem::GetObjectsByTag",
    N_TEST_CONTEXT_EDITOR)
{
    N_TESTS_PERF_START_LATENT_TEST_WORLD
    ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_WorldTest(&FNDynamicRefSubsystemPerfTests_Tags::GetObjectsByTag, true));
    N_TESTS_PERF_FINISH_LATENT_TEST_WORLD
}

N_TEST_PERF(UNDynamicRefSubsystemPerfTests_Tags_GetObjectsByAnyTags,
    "NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystem::GetObjectsByAnyTags",
    N_TEST_CONTEXT_EDITOR)
{
    N_TESTS_PERF_START_LATENT_TEST_WORLD
    ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_WorldTest(&FNDynamicRefSubsystemPerfTests_Tags::GetObjectsByAnyTags, true));
    N_TESTS_PERF_FINISH_LATENT_TEST_WORLD
}

N_TEST_PERF(UNDynamicRefSubsystemPerfTests_Tags_GetObjectsByAllTags,
    "NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystem::GetObjectsByAllTags",
    N_TEST_CONTEXT_EDITOR)
{
    N_TESTS_PERF_START_LATENT_TEST_WORLD
    ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_WorldTest(&FNDynamicRefSubsystemPerfTests_Tags::GetObjectsByAllTags, true));
    N_TESTS_PERF_FINISH_LATENT_TEST_WORLD
}

N_TEST_PERF(UNDynamicRefSubsystemPerfTests_Tags_GetTags,
    "NEXUS::PerfTests::NDynamicRefs::UNDynamicRefSubsystem::GetTags",
    N_TEST_CONTEXT_EDITOR)
{
    N_TESTS_PERF_START_LATENT_TEST_WORLD
    ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_WorldTest(&FNDynamicRefSubsystemPerfTests_Tags::GetTags, true));
    N_TESTS_PERF_FINISH_LATENT_TEST_WORLD
}

#endif //WITH_TESTS
