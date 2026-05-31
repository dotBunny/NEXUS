// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NEditorUtilityWidgetSubsystem.h"
#include "NWidgetState.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness
{
    // Unique key prefix to avoid collisions with real editor config state.
    static const FName WidgetA = TEXT("NUITest_SubsystemWidget_A");
    static const FName WidgetB = TEXT("NUITest_SubsystemWidget_B");
    static const FName TabA    = TEXT("NUITest_TabIdentifier_A");
    static const FName TabB    = TEXT("NUITest_TabIdentifier_B");

    static FNWidgetState MakeState(const bool bValue)
    {
        FNWidgetState S;
        S.AddBoolean(TEXT("bTestKey"), bValue);
        return S;
    }
}

N_TEST_CRITICAL(UNEditorUtilityWidgetSubsystemTests_WidgetState_AddAndHas,
    "NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystem::WidgetState::AddAndHas",
    N_TEST_CONTEXT_EDITOR)
{
    UNEditorUtilityWidgetSubsystem* Subsystem = UNEditorUtilityWidgetSubsystem::Get();
    if (!Subsystem)
    {
        ADD_ERROR("Could not retrieve UNEditorUtilityWidgetSubsystem.");
        return;
    }

    // Clean slate.
    Subsystem->RemoveWidgetState(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA);

    const FNWidgetState State = NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::MakeState(true);
    Subsystem->AddWidgetState(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA, State);

    CHECK_MESSAGE(TEXT("HasWidgetState should return true after AddWidgetState."), Subsystem->HasWidgetState(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA))

    // Cleanup.
    Subsystem->RemoveWidgetState(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA);
}

N_TEST_CRITICAL(UNEditorUtilityWidgetSubsystemTests_WidgetState_GetValue,
    "NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystem::WidgetState::GetValue",
    N_TEST_CONTEXT_EDITOR)
{
    UNEditorUtilityWidgetSubsystem* Subsystem = UNEditorUtilityWidgetSubsystem::Get();
    if (!Subsystem)
    {
        ADD_ERROR("Could not retrieve UNEditorUtilityWidgetSubsystem.");
        return;
    }

    Subsystem->RemoveWidgetState(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetB);

    const FNWidgetState State = NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::MakeState(false);
    Subsystem->AddWidgetState(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetB, State);

    FNWidgetState& Retrieved = Subsystem->GetWidgetState(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetB);
    CHECK_MESSAGE(TEXT("GetWidgetState should return the stored boolean key."), Retrieved.HasBoolean(TEXT("bTestKey")))
    CHECK_FALSE_MESSAGE(TEXT("GetWidgetState should return the correct boolean value (false)."), Retrieved.GetBoolean(TEXT("bTestKey")))

    Subsystem->RemoveWidgetState(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetB);
}

N_TEST_CRITICAL(UNEditorUtilityWidgetSubsystemTests_WidgetState_Remove,
    "NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystem::WidgetState::Remove",
    N_TEST_CONTEXT_EDITOR)
{
    UNEditorUtilityWidgetSubsystem* Subsystem = UNEditorUtilityWidgetSubsystem::Get();
    if (!Subsystem)
    {
        ADD_ERROR("Could not retrieve UNEditorUtilityWidgetSubsystem.");
        return;
    }

    const FNWidgetState State = NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::MakeState(true);
    Subsystem->AddWidgetState(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA, State);
    Subsystem->RemoveWidgetState(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA);

    CHECK_FALSE_MESSAGE(TEXT("HasWidgetState should return false after RemoveWidgetState."), Subsystem->HasWidgetState(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA))
}

N_TEST_HIGH(UNEditorUtilityWidgetSubsystemTests_WidgetState_Update,
    "NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystem::WidgetState::Update",
    N_TEST_CONTEXT_EDITOR)
{
    UNEditorUtilityWidgetSubsystem* Subsystem = UNEditorUtilityWidgetSubsystem::Get();
    if (!Subsystem)
    {
        ADD_ERROR("Could not retrieve UNEditorUtilityWidgetSubsystem.");
        return;
    }

    Subsystem->RemoveWidgetState(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA);

    // Add initial state.
    Subsystem->AddWidgetState(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA,
        NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::MakeState(false));

    // Build replacement state and update.
    FNWidgetState Updated;
    Updated.AddString(TEXT("NewKey"), TEXT("NewValue"));
    Subsystem->UpdateWidgetState(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA, Updated);

    FNWidgetState& Stored = Subsystem->GetWidgetState(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA);
    CHECK_FALSE_MESSAGE(TEXT("Old boolean key should not exist after UpdateWidgetState."), Stored.HasBoolean(TEXT("bTestKey")))
    CHECK_MESSAGE(TEXT("New string key should exist after UpdateWidgetState."), Stored.HasString(TEXT("NewKey")))
    CHECK_EQUALS("New string value should be correct after UpdateWidgetState.",
        Stored.GetString(TEXT("NewKey")), FString(TEXT("NewValue")))

    Subsystem->RemoveWidgetState(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA);
}

N_TEST_CRITICAL(UNEditorUtilityWidgetSubsystemTests_TabIdentifier_SetAndGet,
    "NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystem::TabIdentifier::SetAndGet",
    N_TEST_CONTEXT_EDITOR)
{
    UNEditorUtilityWidgetSubsystem* Subsystem = UNEditorUtilityWidgetSubsystem::Get();
    if (!Subsystem)
    {
        ADD_ERROR("Could not retrieve UNEditorUtilityWidgetSubsystem.");
        return;
    }

    // Clean slate.
    Subsystem->RemoveTabIdentifier(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA);

    Subsystem->SetTabIdentifier(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA,
        NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::TabA);

    const FName Result = Subsystem->GetTabIdentifier(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA);
    CHECK_EQUALS("GetTabIdentifier should return the value set by SetTabIdentifier.",
        Result, NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::TabA)

    Subsystem->RemoveTabIdentifier(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA);
}

N_TEST_CRITICAL(UNEditorUtilityWidgetSubsystemTests_TabIdentifier_Remove,
    "NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystem::TabIdentifier::Remove",
    N_TEST_CONTEXT_EDITOR)
{
    UNEditorUtilityWidgetSubsystem* Subsystem = UNEditorUtilityWidgetSubsystem::Get();
    if (!Subsystem)
    {
        ADD_ERROR("Could not retrieve UNEditorUtilityWidgetSubsystem.");
        return;
    }

    Subsystem->SetTabIdentifier(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetB,
        NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::TabB);
    Subsystem->RemoveTabIdentifier(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetB);

    const FName Result = Subsystem->GetTabIdentifier(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetB);
    CHECK_MESSAGE(TEXT("GetTabIdentifier should return NAME_None after RemoveTabIdentifier."), Result.IsEqual(NAME_None))
}

N_TEST_HIGH(UNEditorUtilityWidgetSubsystemTests_TabIdentifier_UpdateExisting,
    "NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystem::TabIdentifier::UpdateExisting",
    N_TEST_CONTEXT_EDITOR)
{
    // Verifies that calling SetTabIdentifier twice for the same widget updates rather than duplicating the mapping.
    UNEditorUtilityWidgetSubsystem* Subsystem = UNEditorUtilityWidgetSubsystem::Get();
    if (!Subsystem)
    {
        ADD_ERROR("Could not retrieve UNEditorUtilityWidgetSubsystem.");
        return;
    }

    Subsystem->RemoveTabIdentifier(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA);

    Subsystem->SetTabIdentifier(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA,
        NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::TabA);
    Subsystem->SetTabIdentifier(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA,
        NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::TabB);

    const FName Result = Subsystem->GetTabIdentifier(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA);
    CHECK_EQUALS("Second SetTabIdentifier call should update the existing mapping.",
        Result, NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::TabB)

    Subsystem->RemoveTabIdentifier(NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystemHarness::WidgetA);
}

N_TEST_HIGH(UNEditorUtilityWidgetSubsystemTests_TabIdentifier_UnknownReturnsNone,
    "NEXUS::UnitTests::NUI::UNEditorUtilityWidgetSubsystem::TabIdentifier::UnknownReturnsNone",
    N_TEST_CONTEXT_EDITOR)
{
    UNEditorUtilityWidgetSubsystem* Subsystem = UNEditorUtilityWidgetSubsystem::Get();
    if (!Subsystem)
    {
        ADD_ERROR("Could not retrieve UNEditorUtilityWidgetSubsystem.");
        return;
    }

    const FName NeverRegistered = TEXT("NUITest_TabId_NeverRegistered");
    // Ensure it really isn't there.
    Subsystem->RemoveTabIdentifier(NeverRegistered);

    const FName Result = Subsystem->GetTabIdentifier(NeverRegistered);
    CHECK_MESSAGE(TEXT("GetTabIdentifier for an unknown widget should return NAME_None."), Result.IsEqual(NAME_None))
}

#endif //WITH_TESTS
