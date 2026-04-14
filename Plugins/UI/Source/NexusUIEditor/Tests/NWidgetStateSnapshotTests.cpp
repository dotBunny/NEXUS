// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWidgetState.h"
#include "NWidgetStateSnapshot.h"
#include "Tests/TestHarnessAdapter.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::UnitTests::NUI::FNWidgetStateSnapshotHarness
{
    static FNWidgetState MakeState(const FString& Key, const FString& Value)
    {
        FNWidgetState S;
        S.AddString(Key, Value);
        return S;
    }
}

N_TEST_CRITICAL(FNWidgetStateSnapshotTests_AddAndHas,
    "NEXUS::UnitTests::NUI::FNWidgetStateSnapshot::AddAndHas",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetStateSnapshot Snapshot;
    const FNWidgetState State = NEXUS::UnitTests::NUI::FNWidgetStateSnapshotHarness::MakeState(TEXT("Key"), TEXT("Val"));

    const bool bResult = Snapshot.AddWidgetState(TEXT("Widget_A"), State);

    CHECK_MESSAGE(TEXT("AddWidgetState should return true for a new identifier."), bResult)
    CHECK_MESSAGE(TEXT("HasWidgetState should return true after adding."), Snapshot.HasWidgetState(TEXT("Widget_A")))
    CHECK_EQUALS("GetCount should be 1 after adding one state.", Snapshot.GetCount(), 1)
}

N_TEST_CRITICAL(FNWidgetStateSnapshotTests_AddExistingUpdates,
    "NEXUS::UnitTests::NUI::FNWidgetStateSnapshot::AddExistingUpdates",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Verifies that adding a state for an already-registered identifier updates it and returns true.
    FNWidgetStateSnapshot Snapshot;
    FNWidgetState FirstState = NEXUS::UnitTests::NUI::FNWidgetStateSnapshotHarness::MakeState(TEXT("Original"), TEXT("A"));
    Snapshot.AddWidgetState(TEXT("Widget_A"), FirstState);

    FNWidgetState SecondState = NEXUS::UnitTests::NUI::FNWidgetStateSnapshotHarness::MakeState(TEXT("Updated"), TEXT("B"));
    const bool bResult = Snapshot.AddWidgetState(TEXT("Widget_A"), SecondState);

    CHECK_MESSAGE(TEXT("AddWidgetState on existing identifier should return true."), bResult)
    CHECK_EQUALS("GetCount should remain 1 after updating an existing entry.", Snapshot.GetCount(), 1)

    FNWidgetState& Stored = Snapshot.GetWidgetState(TEXT("Widget_A"));
    CHECK_FALSE_MESSAGE(TEXT("Old key should not be present after update."), Stored.HasString(TEXT("Original")))
    CHECK_MESSAGE(TEXT("New key should be present after update."), Stored.HasString(TEXT("Updated")))
}

N_TEST_CRITICAL(FNWidgetStateSnapshotTests_Remove,
    "NEXUS::UnitTests::NUI::FNWidgetStateSnapshot::Remove",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetStateSnapshot Snapshot;
    const FNWidgetState State = NEXUS::UnitTests::NUI::FNWidgetStateSnapshotHarness::MakeState(TEXT("K"), TEXT("V"));
    Snapshot.AddWidgetState(TEXT("Widget_Remove"), State);

    const bool bResult = Snapshot.RemoveWidgetState(TEXT("Widget_Remove"));

    CHECK_MESSAGE(TEXT("RemoveWidgetState should return true when found."), bResult)
    CHECK_FALSE_MESSAGE(TEXT("HasWidgetState should return false after removal."), Snapshot.HasWidgetState(TEXT("Widget_Remove")))
    CHECK_EQUALS("GetCount should be 0 after removing the only entry.", Snapshot.GetCount(), 0)
}

N_TEST_HIGH(FNWidgetStateSnapshotTests_RemoveUnknown,
    "NEXUS::UnitTests::NUI::FNWidgetStateSnapshot::RemoveUnknown",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetStateSnapshot Snapshot;
    const bool bResult = Snapshot.RemoveWidgetState(TEXT("NonExistent"));

    CHECK_FALSE_MESSAGE(TEXT("RemoveWidgetState should return false for an unknown identifier."), bResult)
    CHECK_EQUALS("GetCount should remain 0 when removing a non-existent entry.", Snapshot.GetCount(), 0)
}

N_TEST_CRITICAL(FNWidgetStateSnapshotTests_Update,
    "NEXUS::UnitTests::NUI::FNWidgetStateSnapshot::Update",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetStateSnapshot Snapshot;
    Snapshot.AddWidgetState(TEXT("Widget_U"), NEXUS::UnitTests::NUI::FNWidgetStateSnapshotHarness::MakeState(TEXT("Before"), TEXT("1")));

    FNWidgetState NewState = NEXUS::UnitTests::NUI::FNWidgetStateSnapshotHarness::MakeState(TEXT("After"), TEXT("2"));
    const bool bResult = Snapshot.UpdateWidgetState(TEXT("Widget_U"), NewState);

    CHECK_MESSAGE(TEXT("UpdateWidgetState should return true when the identifier exists."), bResult)

    FNWidgetState& Stored = Snapshot.GetWidgetState(TEXT("Widget_U"));
    CHECK_FALSE_MESSAGE(TEXT("Old key should not be present after UpdateWidgetState."), Stored.HasString(TEXT("Before")))
    CHECK_MESSAGE(TEXT("New key should be present after UpdateWidgetState."), Stored.HasString(TEXT("After")))
}

N_TEST_HIGH(FNWidgetStateSnapshotTests_UpdateUnknown,
    "NEXUS::UnitTests::NUI::FNWidgetStateSnapshot::UpdateUnknown",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Verifies that UpdateWidgetState returns false and does not add an entry when the identifier is not registered.
    FNWidgetStateSnapshot Snapshot;
    const FNWidgetState State = NEXUS::UnitTests::NUI::FNWidgetStateSnapshotHarness::MakeState(TEXT("K"), TEXT("V"));

    // UpdateWidgetState logs a warning for unknown identifiers; suppress it here.
    AddExpectedMessage("Failed to update widget", ELogVerbosity::Type::Warning);

    const bool bResult = Snapshot.UpdateWidgetState(TEXT("NonExistent"), State);

    CHECK_FALSE_MESSAGE(TEXT("UpdateWidgetState should return false for an unknown identifier."), bResult)
    CHECK_EQUALS("GetCount should remain 0 when updating a non-existent entry.", Snapshot.GetCount(), 0)
}

N_TEST_HIGH(FNWidgetStateSnapshotTests_GetWidgetState,
    "NEXUS::UnitTests::NUI::FNWidgetStateSnapshot::GetWidgetState",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetStateSnapshot Snapshot;
    FNWidgetState State;
    State.AddBoolean(TEXT("bFlag"), true);
    State.AddFloat(TEXT("Scale"), 2.5f);
    Snapshot.AddWidgetState(TEXT("Widget_Get"), State);

    FNWidgetState& Retrieved = Snapshot.GetWidgetState(TEXT("Widget_Get"));

    CHECK_MESSAGE(TEXT("Retrieved state should contain the boolean key."), Retrieved.HasBoolean(TEXT("bFlag")))
    CHECK_MESSAGE(TEXT("Retrieved boolean value should match."), Retrieved.GetBoolean(TEXT("bFlag")))
    CHECK_EQUALS("Retrieved float value should match.", Retrieved.GetFloat(TEXT("Scale")), 2.5f)
}

N_TEST_HIGH(FNWidgetStateSnapshotTests_CountAndClear,
    "NEXUS::UnitTests::NUI::FNWidgetStateSnapshot::CountAndClear",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetStateSnapshot Snapshot;
    const FNWidgetState S = NEXUS::UnitTests::NUI::FNWidgetStateSnapshotHarness::MakeState(TEXT("K"), TEXT("V"));

    Snapshot.AddWidgetState(TEXT("A"), S);
    Snapshot.AddWidgetState(TEXT("B"), S);
    Snapshot.AddWidgetState(TEXT("C"), S);

    CHECK_EQUALS("GetCount should be 3 after adding three entries.", Snapshot.GetCount(), 3)

    Snapshot.Clear();

    CHECK_EQUALS("GetCount should be 0 after Clear.", Snapshot.GetCount(), 0)
    CHECK_FALSE_MESSAGE(TEXT("HasWidgetState should return false after Clear."), Snapshot.HasWidgetState(TEXT("A")))
}

N_TEST_HIGH(FNWidgetStateSnapshotTests_ArraySyncAfterRemove,
    "NEXUS::UnitTests::NUI::FNWidgetStateSnapshot::ArraySyncAfterRemove",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Verifies that removing a middle entry keeps remaining entries accessible with correct state after re-indexing.
    FNWidgetStateSnapshot Snapshot;
    Snapshot.AddWidgetState(TEXT("First"),  NEXUS::UnitTests::NUI::FNWidgetStateSnapshotHarness::MakeState(TEXT("Idx"), TEXT("0")));
    Snapshot.AddWidgetState(TEXT("Middle"), NEXUS::UnitTests::NUI::FNWidgetStateSnapshotHarness::MakeState(TEXT("Idx"), TEXT("1")));
    Snapshot.AddWidgetState(TEXT("Last"),   NEXUS::UnitTests::NUI::FNWidgetStateSnapshotHarness::MakeState(TEXT("Idx"), TEXT("2")));

    Snapshot.RemoveWidgetState(TEXT("Middle"));

    CHECK_EQUALS("GetCount should be 2 after removing the middle entry.", Snapshot.GetCount(), 2)
    CHECK_MESSAGE(TEXT("First entry should still exist."), Snapshot.HasWidgetState(TEXT("First")))
    CHECK_MESSAGE(TEXT("Last entry should still exist."), Snapshot.HasWidgetState(TEXT("Last")))
    CHECK_FALSE_MESSAGE(TEXT("Middle entry should no longer exist."), Snapshot.HasWidgetState(TEXT("Middle")))

    // Verify values are still accessible correctly after re-indexing.
    CHECK_EQUALS("First entry state should still be intact.",
        Snapshot.GetWidgetState(TEXT("First")).GetString(TEXT("Idx")), FString(TEXT("0")))
    CHECK_EQUALS("Last entry state should still be intact.",
        Snapshot.GetWidgetState(TEXT("Last")).GetString(TEXT("Idx")), FString(TEXT("2")))
}

N_TEST_HIGH(FNWidgetStateSnapshotTests_GetIdentifierIndex,
    "NEXUS::UnitTests::NUI::FNWidgetStateSnapshot::GetIdentifierIndex",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetStateSnapshot Snapshot;
    const FNWidgetState S = NEXUS::UnitTests::NUI::FNWidgetStateSnapshotHarness::MakeState(TEXT("K"), TEXT("V"));
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
