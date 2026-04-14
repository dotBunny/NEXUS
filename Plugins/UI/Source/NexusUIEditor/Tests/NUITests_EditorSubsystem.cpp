// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NEditorUtilityWidgetSubsystem.h"
#include "NWidgetState.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

namespace NEXUS::UITests::EditorSubsystem
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

N_TEST_CRITICAL(FNUITests_EditorSubsystem_WidgetState_AddAndHas,
    "NEXUS::UnitTests::NUI::EditorSubsystem::WidgetState::AddAndHas",
    N_TEST_CONTEXT_EDITOR)
{
    UNEditorUtilityWidgetSubsystem* Subsystem = UNEditorUtilityWidgetSubsystem::Get();
    if (!Subsystem)
    {
        ADD_ERROR("Could not retrieve UNEditorUtilityWidgetSubsystem.");
        return;
    }

    // Clean slate.
    Subsystem->RemoveWidgetState(NEXUS::UITests::EditorSubsystem::WidgetA);

    const FNWidgetState State = NEXUS::UITests::EditorSubsystem::MakeState(true);
    Subsystem->AddWidgetState(NEXUS::UITests::EditorSubsystem::WidgetA, State);

    CHECK_MESSAGE("HasWidgetState should return true after AddWidgetState.", Subsystem->HasWidgetState(NEXUS::UITests::EditorSubsystem::WidgetA))

    // Cleanup.
    Subsystem->RemoveWidgetState(NEXUS::UITests::EditorSubsystem::WidgetA);
}

N_TEST_CRITICAL(FNUITests_EditorSubsystem_WidgetState_GetValue,
    "NEXUS::UnitTests::NUI::EditorSubsystem::WidgetState::GetValue",
    N_TEST_CONTEXT_EDITOR)
{
    UNEditorUtilityWidgetSubsystem* Subsystem = UNEditorUtilityWidgetSubsystem::Get();
    if (!Subsystem)
    {
        ADD_ERROR("Could not retrieve UNEditorUtilityWidgetSubsystem.");
        return;
    }

    Subsystem->RemoveWidgetState(NEXUS::UITests::EditorSubsystem::WidgetB);

    const FNWidgetState State = NEXUS::UITests::EditorSubsystem::MakeState(false);
    Subsystem->AddWidgetState(NEXUS::UITests::EditorSubsystem::WidgetB, State);

    FNWidgetState& Retrieved = Subsystem->GetWidgetState(NEXUS::UITests::EditorSubsystem::WidgetB);
    CHECK_MESSAGE("GetWidgetState should return the stored boolean key.", Retrieved.HasBoolean(TEXT("bTestKey")))
    CHECK_FALSE_MESSAGE("GetWidgetState should return the correct boolean value (false).", Retrieved.GetBoolean(TEXT("bTestKey")))

    Subsystem->RemoveWidgetState(NEXUS::UITests::EditorSubsystem::WidgetB);
}

N_TEST_CRITICAL(FNUITests_EditorSubsystem_WidgetState_Remove,
    "NEXUS::UnitTests::NUI::EditorSubsystem::WidgetState::Remove",
    N_TEST_CONTEXT_EDITOR)
{
    UNEditorUtilityWidgetSubsystem* Subsystem = UNEditorUtilityWidgetSubsystem::Get();
    if (!Subsystem)
    {
        ADD_ERROR("Could not retrieve UNEditorUtilityWidgetSubsystem.");
        return;
    }

    const FNWidgetState State = NEXUS::UITests::EditorSubsystem::MakeState(true);
    Subsystem->AddWidgetState(NEXUS::UITests::EditorSubsystem::WidgetA, State);
    Subsystem->RemoveWidgetState(NEXUS::UITests::EditorSubsystem::WidgetA);

    CHECK_FALSE_MESSAGE("HasWidgetState should return false after RemoveWidgetState.", Subsystem->HasWidgetState(NEXUS::UITests::EditorSubsystem::WidgetA))
}

N_TEST_HIGH(FNUITests_EditorSubsystem_WidgetState_Update,
    "NEXUS::UnitTests::NUI::EditorSubsystem::WidgetState::Update",
    N_TEST_CONTEXT_EDITOR)
{
    UNEditorUtilityWidgetSubsystem* Subsystem = UNEditorUtilityWidgetSubsystem::Get();
    if (!Subsystem)
    {
        ADD_ERROR("Could not retrieve UNEditorUtilityWidgetSubsystem.");
        return;
    }

    Subsystem->RemoveWidgetState(NEXUS::UITests::EditorSubsystem::WidgetA);

    // Add initial state.
    Subsystem->AddWidgetState(NEXUS::UITests::EditorSubsystem::WidgetA,
        NEXUS::UITests::EditorSubsystem::MakeState(false));

    // Build replacement state and update.
    FNWidgetState Updated;
    Updated.AddString(TEXT("NewKey"), TEXT("NewValue"));
    Subsystem->UpdateWidgetState(NEXUS::UITests::EditorSubsystem::WidgetA, Updated);

    FNWidgetState& Stored = Subsystem->GetWidgetState(NEXUS::UITests::EditorSubsystem::WidgetA);
    CHECK_FALSE_MESSAGE("Old boolean key should not exist after UpdateWidgetState.", Stored.HasBoolean(TEXT("bTestKey")))
    CHECK_MESSAGE("New string key should exist after UpdateWidgetState.", Stored.HasString(TEXT("NewKey")))
    CHECK_EQUALS("New string value should be correct after UpdateWidgetState.",
        Stored.GetString(TEXT("NewKey")), FString(TEXT("NewValue")))

    Subsystem->RemoveWidgetState(NEXUS::UITests::EditorSubsystem::WidgetA);
}

N_TEST_CRITICAL(FNUITests_EditorSubsystem_TabIdentifier_SetAndGet,
    "NEXUS::UnitTests::NUI::EditorSubsystem::TabIdentifier::SetAndGet",
    N_TEST_CONTEXT_EDITOR)
{
    UNEditorUtilityWidgetSubsystem* Subsystem = UNEditorUtilityWidgetSubsystem::Get();
    if (!Subsystem)
    {
        ADD_ERROR("Could not retrieve UNEditorUtilityWidgetSubsystem.");
        return;
    }

    // Clean slate.
    Subsystem->RemoveTabIdentifier(NEXUS::UITests::EditorSubsystem::WidgetA);

    Subsystem->SetTabIdentifier(NEXUS::UITests::EditorSubsystem::WidgetA,
        NEXUS::UITests::EditorSubsystem::TabA);

    const FName Result = Subsystem->GetTabIdentifier(NEXUS::UITests::EditorSubsystem::WidgetA);
    CHECK_EQUALS("GetTabIdentifier should return the value set by SetTabIdentifier.",
        Result, NEXUS::UITests::EditorSubsystem::TabA)

    Subsystem->RemoveTabIdentifier(NEXUS::UITests::EditorSubsystem::WidgetA);
}

N_TEST_CRITICAL(FNUITests_EditorSubsystem_TabIdentifier_Remove,
    "NEXUS::UnitTests::NUI::EditorSubsystem::TabIdentifier::Remove",
    N_TEST_CONTEXT_EDITOR)
{
    UNEditorUtilityWidgetSubsystem* Subsystem = UNEditorUtilityWidgetSubsystem::Get();
    if (!Subsystem)
    {
        ADD_ERROR("Could not retrieve UNEditorUtilityWidgetSubsystem.");
        return;
    }

    Subsystem->SetTabIdentifier(NEXUS::UITests::EditorSubsystem::WidgetB,
        NEXUS::UITests::EditorSubsystem::TabB);
    Subsystem->RemoveTabIdentifier(NEXUS::UITests::EditorSubsystem::WidgetB);

    const FName Result = Subsystem->GetTabIdentifier(NEXUS::UITests::EditorSubsystem::WidgetB);
    CHECK_MESSAGE("GetTabIdentifier should return NAME_None after RemoveTabIdentifier.", Result.IsEqual(NAME_None))
}

N_TEST_HIGH(FNUITests_EditorSubsystem_TabIdentifier_UpdateExisting,
    "NEXUS::UnitTests::NUI::EditorSubsystem::TabIdentifier::UpdateExisting",
    N_TEST_CONTEXT_EDITOR)
{
    // Calling SetTabIdentifier again for the same widget should update, not duplicate.
    UNEditorUtilityWidgetSubsystem* Subsystem = UNEditorUtilityWidgetSubsystem::Get();
    if (!Subsystem)
    {
        ADD_ERROR("Could not retrieve UNEditorUtilityWidgetSubsystem.");
        return;
    }

    Subsystem->RemoveTabIdentifier(NEXUS::UITests::EditorSubsystem::WidgetA);

    Subsystem->SetTabIdentifier(NEXUS::UITests::EditorSubsystem::WidgetA,
        NEXUS::UITests::EditorSubsystem::TabA);
    Subsystem->SetTabIdentifier(NEXUS::UITests::EditorSubsystem::WidgetA,
        NEXUS::UITests::EditorSubsystem::TabB);

    const FName Result = Subsystem->GetTabIdentifier(NEXUS::UITests::EditorSubsystem::WidgetA);
    CHECK_EQUALS("Second SetTabIdentifier call should update the existing mapping.",
        Result, NEXUS::UITests::EditorSubsystem::TabB)

    Subsystem->RemoveTabIdentifier(NEXUS::UITests::EditorSubsystem::WidgetA);
}

N_TEST_HIGH(FNUITests_EditorSubsystem_TabIdentifier_UnknownReturnsNone,
    "NEXUS::UnitTests::NUI::EditorSubsystem::TabIdentifier::UnknownReturnsNone",
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
    CHECK_MESSAGE("GetTabIdentifier for an unknown widget should return NAME_None.", Result.IsEqual(NAME_None))
}

#endif //WITH_TESTS
