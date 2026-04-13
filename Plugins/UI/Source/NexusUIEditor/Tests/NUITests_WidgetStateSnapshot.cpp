// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWidgetState.h"
#include "NWidgetStateSnapshot.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

// ─── Helpers ──────────────────────────────────────────────────────────────────

namespace NEXUS::UITests::WidgetStateSnapshot
{
    static FNWidgetState MakeState(const FString& Key, const FString& Value)
    {
        FNWidgetState S;
        S.AddString(Key, Value);
        return S;
    }
}

// ─── Add / Has ────────────────────────────────────────────────────────────────

N_TEST_CRITICAL(FNUITests_WidgetStateSnapshot_AddAndHas,
    "NEXUS::UnitTests::NUI::WidgetStateSnapshot::AddAndHas",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetStateSnapshot Snapshot;
    const FNWidgetState State = NEXUS::UITests::WidgetStateSnapshot::MakeState(TEXT("Key"), TEXT("Val"));

    const bool bResult = Snapshot.AddWidgetState(TEXT("Widget_A"), State);

    CHECK_MESSAGE("AddWidgetState should return true for a new identifier.", bResult)
    CHECK_MESSAGE("HasWidgetState should return true after adding.", Snapshot.HasWidgetState(TEXT("Widget_A")))
    CHECK_EQUALS("GetCount should be 1 after adding one state.", Snapshot.GetCount(), 1)
}

N_TEST_CRITICAL(FNUITests_WidgetStateSnapshot_AddExistingUpdates,
    "NEXUS::UnitTests::NUI::WidgetStateSnapshot::AddExistingUpdates",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Adding to an already-registered identifier updates its state and returns true.
    FNWidgetStateSnapshot Snapshot;
    FNWidgetState FirstState = NEXUS::UITests::WidgetStateSnapshot::MakeState(TEXT("Original"), TEXT("A"));
    Snapshot.AddWidgetState(TEXT("Widget_A"), FirstState);

    FNWidgetState SecondState = NEXUS::UITests::WidgetStateSnapshot::MakeState(TEXT("Updated"), TEXT("B"));
    const bool bResult = Snapshot.AddWidgetState(TEXT("Widget_A"), SecondState);

    CHECK_MESSAGE("AddWidgetState on existing identifier should return true.", bResult)
    CHECK_EQUALS("GetCount should remain 1 after updating an existing entry.", Snapshot.GetCount(), 1)

    FNWidgetState& Stored = Snapshot.GetWidgetState(TEXT("Widget_A"));
    CHECK_FALSE_MESSAGE("Old key should not be present after update.", Stored.HasString(TEXT("Original")))
    CHECK_MESSAGE("New key should be present after update.", Stored.HasString(TEXT("Updated")))
}

// ─── Remove ───────────────────────────────────────────────────────────────────

N_TEST_CRITICAL(FNUITests_WidgetStateSnapshot_Remove,
    "NEXUS::UnitTests::NUI::WidgetStateSnapshot::Remove",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetStateSnapshot Snapshot;
    const FNWidgetState State = NEXUS::UITests::WidgetStateSnapshot::MakeState(TEXT("K"), TEXT("V"));
    Snapshot.AddWidgetState(TEXT("Widget_Remove"), State);

    const bool bResult = Snapshot.RemoveWidgetState(TEXT("Widget_Remove"));

    CHECK_MESSAGE("RemoveWidgetState should return true when found.", bResult)
    CHECK_FALSE_MESSAGE("HasWidgetState should return false after removal.", Snapshot.HasWidgetState(TEXT("Widget_Remove")))
    CHECK_EQUALS("GetCount should be 0 after removing the only entry.", Snapshot.GetCount(), 0)
}

N_TEST_HIGH(FNUITests_WidgetStateSnapshot_RemoveUnknown,
    "NEXUS::UnitTests::NUI::WidgetStateSnapshot::RemoveUnknown",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetStateSnapshot Snapshot;
    const bool bResult = Snapshot.RemoveWidgetState(TEXT("NonExistent"));

    CHECK_FALSE_MESSAGE("RemoveWidgetState should return false for an unknown identifier.", bResult)
    CHECK_EQUALS("GetCount should remain 0 when removing a non-existent entry.", Snapshot.GetCount(), 0)
}

// ─── Update ───────────────────────────────────────────────────────────────────

N_TEST_CRITICAL(FNUITests_WidgetStateSnapshot_Update,
    "NEXUS::UnitTests::NUI::WidgetStateSnapshot::Update",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetStateSnapshot Snapshot;
    Snapshot.AddWidgetState(TEXT("Widget_U"), NEXUS::UITests::WidgetStateSnapshot::MakeState(TEXT("Before"), TEXT("1")));

    FNWidgetState NewState = NEXUS::UITests::WidgetStateSnapshot::MakeState(TEXT("After"), TEXT("2"));
    const bool bResult = Snapshot.UpdateWidgetState(TEXT("Widget_U"), NewState);

    CHECK_MESSAGE("UpdateWidgetState should return true when the identifier exists.", bResult)

    FNWidgetState& Stored = Snapshot.GetWidgetState(TEXT("Widget_U"));
    CHECK_FALSE_MESSAGE("Old key should not be present after UpdateWidgetState.", Stored.HasString(TEXT("Before")))
    CHECK_MESSAGE("New key should be present after UpdateWidgetState.", Stored.HasString(TEXT("After")))
}

N_TEST_HIGH(FNUITests_WidgetStateSnapshot_UpdateUnknown,
    "NEXUS::UnitTests::NUI::WidgetStateSnapshot::UpdateUnknown",
    N_TEST_CONTEXT_ANYWHERE)
{
    // UpdateWidgetState on a non-existent identifier should return false and not add.
    FNWidgetStateSnapshot Snapshot;
    const FNWidgetState State = NEXUS::UITests::WidgetStateSnapshot::MakeState(TEXT("K"), TEXT("V"));

	// We're going to throw a warning it is expected
	AddExpectedMessage("Failed to update widget", ELogVerbosity::Type::Warning);
	
    const bool bResult = Snapshot.UpdateWidgetState(TEXT("NonExistent"), State);

    CHECK_FALSE_MESSAGE("UpdateWidgetState should return false for an unknown identifier.", bResult)
    CHECK_EQUALS("GetCount should remain 0 when updating a non-existent entry.", Snapshot.GetCount(), 0)
}

// ─── GetState ────────────────────────────────────────────────────────────────

N_TEST_HIGH(FNUITests_WidgetStateSnapshot_GetWidgetState,
    "NEXUS::UnitTests::NUI::WidgetStateSnapshot::GetWidgetState",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetStateSnapshot Snapshot;
    FNWidgetState State;
    State.AddBoolean(TEXT("bFlag"), true);
    State.AddFloat(TEXT("Scale"), 2.5f);
    Snapshot.AddWidgetState(TEXT("Widget_Get"), State);

    FNWidgetState& Retrieved = Snapshot.GetWidgetState(TEXT("Widget_Get"));

    CHECK_MESSAGE("Retrieved state should contain the boolean key.", Retrieved.HasBoolean(TEXT("bFlag")))
    CHECK_MESSAGE("Retrieved boolean value should match.", Retrieved.GetBoolean(TEXT("bFlag")))
    CHECK_EQUALS("Retrieved float value should match.", Retrieved.GetFloat(TEXT("Scale")), 2.5f)
}

// ─── Count / Clear ────────────────────────────────────────────────────────────

N_TEST_HIGH(FNUITests_WidgetStateSnapshot_CountAndClear,
    "NEXUS::UnitTests::NUI::WidgetStateSnapshot::CountAndClear",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetStateSnapshot Snapshot;
    const FNWidgetState S = NEXUS::UITests::WidgetStateSnapshot::MakeState(TEXT("K"), TEXT("V"));

    Snapshot.AddWidgetState(TEXT("A"), S);
    Snapshot.AddWidgetState(TEXT("B"), S);
    Snapshot.AddWidgetState(TEXT("C"), S);

    CHECK_EQUALS("GetCount should be 3 after adding three entries.", Snapshot.GetCount(), 3)

    Snapshot.Clear();

    CHECK_EQUALS("GetCount should be 0 after Clear.", Snapshot.GetCount(), 0)
    CHECK_FALSE_MESSAGE("HasWidgetState should return false after Clear.", Snapshot.HasWidgetState(TEXT("A")))
}

// ─── Array Synchronisation ────────────────────────────────────────────────────

N_TEST_HIGH(FNUITests_WidgetStateSnapshot_ArraySyncAfterRemove,
    "NEXUS::UnitTests::NUI::WidgetStateSnapshot::ArraySyncAfterRemove",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Add three entries, remove the middle one, then verify the remaining
    // two entries are still accessible with the correct states.
    FNWidgetStateSnapshot Snapshot;
    Snapshot.AddWidgetState(TEXT("First"),  NEXUS::UITests::WidgetStateSnapshot::MakeState(TEXT("Idx"), TEXT("0")));
    Snapshot.AddWidgetState(TEXT("Middle"), NEXUS::UITests::WidgetStateSnapshot::MakeState(TEXT("Idx"), TEXT("1")));
    Snapshot.AddWidgetState(TEXT("Last"),   NEXUS::UITests::WidgetStateSnapshot::MakeState(TEXT("Idx"), TEXT("2")));

    Snapshot.RemoveWidgetState(TEXT("Middle"));

    CHECK_EQUALS("GetCount should be 2 after removing the middle entry.", Snapshot.GetCount(), 2)
    CHECK_MESSAGE("First entry should still exist.", Snapshot.HasWidgetState(TEXT("First")))
    CHECK_MESSAGE("Last entry should still exist.", Snapshot.HasWidgetState(TEXT("Last")))
    CHECK_FALSE_MESSAGE("Middle entry should no longer exist.", Snapshot.HasWidgetState(TEXT("Middle")))

    // Verify values are still accessible correctly after re-indexing.
    CHECK_EQUALS("First entry state should still be intact.",
        Snapshot.GetWidgetState(TEXT("First")).GetString(TEXT("Idx")), FString(TEXT("0")))
    CHECK_EQUALS("Last entry state should still be intact.",
        Snapshot.GetWidgetState(TEXT("Last")).GetString(TEXT("Idx")), FString(TEXT("2")))
}

// ─── GetIdentifierIndex ───────────────────────────────────────────────────────

N_TEST_HIGH(FNUITests_WidgetStateSnapshot_GetIdentifierIndex,
    "NEXUS::UnitTests::NUI::WidgetStateSnapshot::GetIdentifierIndex",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetStateSnapshot Snapshot;
    const FNWidgetState S = NEXUS::UITests::WidgetStateSnapshot::MakeState(TEXT("K"), TEXT("V"));
    Snapshot.AddWidgetState(TEXT("Alpha"), S);
    Snapshot.AddWidgetState(TEXT("Beta"), S);

    const int32 AlphaIndex = Snapshot.GetIdentifierIndex(TEXT("Alpha"));
    const int32 BetaIndex = Snapshot.GetIdentifierIndex(TEXT("Beta"));
    const int32 MissingIndex = Snapshot.GetIdentifierIndex(TEXT("Missing"));

    CHECK_EQUALS("Alpha should be at index 0.", AlphaIndex, 0)
    CHECK_EQUALS("Beta should be at index 1.", BetaIndex, 1)
    CHECK_EQUALS("Missing identifier should return INDEX_NONE.", MissingIndex, INDEX_NONE)
}

#endif //WITH_TESTS
