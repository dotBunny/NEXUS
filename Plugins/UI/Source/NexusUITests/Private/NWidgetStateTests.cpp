// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "NWidgetState.h"
#include "Tests/TestHarnessAdapter.h"
#include "Macros/NTestMacros.h"

N_TEST_CRITICAL(FNWidgetStateTests_Boolean_AddAndGet,
    "NEXUS::UnitTests::NUI::FNWidgetState::Boolean::AddAndGet",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;
    State.AddBoolean(TEXT("bMyKey"), true);

    CHECK_MESSAGE(TEXT("Has should be true after AddBoolean."), State.HasBoolean(TEXT("bMyKey")))
    CHECK_MESSAGE(TEXT("GetBoolean should return the stored value."), State.GetBoolean(TEXT("bMyKey")))
}

N_TEST_CRITICAL(FNWidgetStateTests_Boolean_DefaultValue,
    "NEXUS::UnitTests::NUI::FNWidgetState::Boolean::DefaultValue",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;

    CHECK_FALSE_MESSAGE(TEXT("Has should return false for an unknown key."), State.HasBoolean(TEXT("bMissing")))
    CHECK_FALSE_MESSAGE(TEXT("GetBoolean should return the specified default (false)."), State.GetBoolean(TEXT("bMissing"), false))
    CHECK_MESSAGE(TEXT("GetBoolean should return the specified default (true)."), State.GetBoolean(TEXT("bMissing"), true))
}

N_TEST_CRITICAL(FNWidgetStateTests_Boolean_SetUpdatesExisting,
    "NEXUS::UnitTests::NUI::FNWidgetState::Boolean::SetUpdatesExisting",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;
    State.AddBoolean(TEXT("bToggle"), false);
    State.SetBoolean(TEXT("bToggle"), true);

    CHECK_MESSAGE(TEXT("SetBoolean should update an existing key's value."), State.GetBoolean(TEXT("bToggle")))
}

N_TEST_HIGH(FNWidgetStateTests_Boolean_SetAddsNew,
    "NEXUS::UnitTests::NUI::FNWidgetState::Boolean::SetAddsNew",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Verifies that SetBoolean implicitly creates an entry when the key does not yet exist.
    FNWidgetState State;
    State.SetBoolean(TEXT("bNew"), true);

    CHECK_MESSAGE(TEXT("SetBoolean on a new key should create the entry."), State.HasBoolean(TEXT("bNew")))
    CHECK_MESSAGE(TEXT("SetBoolean on a new key should store the correct value."), State.GetBoolean(TEXT("bNew")))
}

N_TEST_HIGH(FNWidgetStateTests_Boolean_Remove,
    "NEXUS::UnitTests::NUI::FNWidgetState::Boolean::Remove",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;
    State.AddBoolean(TEXT("bRemoveMe"), true);
    State.RemoveBoolean(TEXT("bRemoveMe"));

    CHECK_FALSE_MESSAGE(TEXT("HasBoolean should return false after RemoveBoolean."), State.HasBoolean(TEXT("bRemoveMe")))
}

N_TEST_HIGH(FNWidgetStateTests_Boolean_RemoveUnknown,
    "NEXUS::UnitTests::NUI::FNWidgetState::Boolean::RemoveUnknown",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Verifies that RemoveBoolean is a no-op and does not crash when the key does not exist.
    FNWidgetState State;
    State.RemoveBoolean(TEXT("bNonExistent"));
    CHECK_FALSE_MESSAGE(TEXT("HasBoolean should remain false after removing a non-existent key."), State.HasBoolean(TEXT("bNonExistent")))
}

N_TEST_HIGH(FNWidgetStateTests_Boolean_Clear,
    "NEXUS::UnitTests::NUI::FNWidgetState::Boolean::Clear",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;
    State.AddBoolean(TEXT("bA"), true);
    State.AddBoolean(TEXT("bB"), false);
    State.ClearBooleans();

    CHECK_FALSE_MESSAGE(TEXT("HasBoolean should return false after ClearBooleans."), State.HasBoolean(TEXT("bA")))
    CHECK_FALSE_MESSAGE(TEXT("HasBoolean should return false after ClearBooleans."), State.HasBoolean(TEXT("bB")))
}

N_TEST_CRITICAL(FNWidgetStateTests_Float_AddAndGet,
    "NEXUS::UnitTests::NUI::FNWidgetState::Float::AddAndGet",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;
    State.AddFloat(TEXT("Volume"), 0.75f);

    CHECK_MESSAGE(TEXT("Has should be true after AddFloat."), State.HasFloat(TEXT("Volume")))
    CHECK_EQUALS("GetFloat should return the stored value.", State.GetFloat(TEXT("Volume")), 0.75f)
}

N_TEST_CRITICAL(FNWidgetStateTests_Float_DefaultValue,
    "NEXUS::UnitTests::NUI::FNWidgetState::Float::DefaultValue",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;

    CHECK_FALSE_MESSAGE(TEXT("Has should return false for an unknown float key."), State.HasFloat(TEXT("Missing")))
    CHECK_EQUALS("GetFloat default should be 0.0 when not specified.", State.GetFloat(TEXT("Missing")), 0.f)
    CHECK_EQUALS("GetFloat should return the provided default.", State.GetFloat(TEXT("Missing"), -1.f), -1.f)
}

N_TEST_CRITICAL(FNWidgetStateTests_Float_SetUpdatesExisting,
    "NEXUS::UnitTests::NUI::FNWidgetState::Float::SetUpdatesExisting",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;
    State.AddFloat(TEXT("Brightness"), 0.5f);
    State.SetFloat(TEXT("Brightness"), 1.0f);

    CHECK_EQUALS("SetFloat should overwrite an existing key's value.", State.GetFloat(TEXT("Brightness")), 1.0f)
}

N_TEST_HIGH(FNWidgetStateTests_Float_SetAddsNew,
    "NEXUS::UnitTests::NUI::FNWidgetState::Float::SetAddsNew",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;
    State.SetFloat(TEXT("NewFloat"), 3.14f);

    CHECK_MESSAGE(TEXT("SetFloat on a new key should create the entry."), State.HasFloat(TEXT("NewFloat")))
    CHECK_EQUALS("SetFloat on a new key should store the correct value.", State.GetFloat(TEXT("NewFloat")), 3.14f)
}

N_TEST_HIGH(FNWidgetStateTests_Float_Remove,
    "NEXUS::UnitTests::NUI::FNWidgetState::Float::Remove",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;
    State.AddFloat(TEXT("Gamma"), 2.2f);
    State.RemoveFloat(TEXT("Gamma"));

    CHECK_FALSE_MESSAGE(TEXT("HasFloat should return false after RemoveFloat."), State.HasFloat(TEXT("Gamma")))
}

N_TEST_HIGH(FNWidgetStateTests_Float_Clear,
    "NEXUS::UnitTests::NUI::FNWidgetState::Float::Clear",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;
    State.AddFloat(TEXT("A"), 1.f);
    State.AddFloat(TEXT("B"), 2.f);
    State.ClearFloats();

    CHECK_FALSE_MESSAGE(TEXT("HasFloat should return false after ClearFloats."), State.HasFloat(TEXT("A")))
    CHECK_FALSE_MESSAGE(TEXT("HasFloat should return false after ClearFloats."), State.HasFloat(TEXT("B")))
}

N_TEST_CRITICAL(FNWidgetStateTests_String_AddAndGet,
    "NEXUS::UnitTests::NUI::FNWidgetState::String::AddAndGet",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;
    State.AddString(TEXT("PlayerName"), TEXT("Alice"));

    CHECK_MESSAGE(TEXT("Has should be true after AddString."), State.HasString(TEXT("PlayerName")))
    CHECK_EQUALS("GetString should return the stored value.", State.GetString(TEXT("PlayerName")), FString(TEXT("Alice")))
}

N_TEST_CRITICAL(FNWidgetStateTests_String_DefaultValue,
    "NEXUS::UnitTests::NUI::FNWidgetState::String::DefaultValue",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;

    CHECK_FALSE_MESSAGE(TEXT("Has should return false for an unknown string key."), State.HasString(TEXT("Missing")))
    CHECK_EQUALS("GetString for a missing key should return an empty string.", State.GetString(TEXT("Missing")), FString())
}

N_TEST_CRITICAL(FNWidgetStateTests_String_SetUpdatesExisting,
    "NEXUS::UnitTests::NUI::FNWidgetState::String::SetUpdatesExisting",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;
    State.AddString(TEXT("Mode"), TEXT("Normal"));
    State.SetString(TEXT("Mode"), TEXT("Hard"));

    CHECK_EQUALS("SetString should overwrite an existing key's value.", State.GetString(TEXT("Mode")), FString(TEXT("Hard")))
}

N_TEST_HIGH(FNWidgetStateTests_String_SetAddsNew,
    "NEXUS::UnitTests::NUI::FNWidgetState::String::SetAddsNew",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;
    State.SetString(TEXT("NewKey"), TEXT("NewValue"));

    CHECK_MESSAGE(TEXT("SetString on a new key should create the entry."), State.HasString(TEXT("NewKey")))
    CHECK_EQUALS("SetString on a new key should store the correct value.", State.GetString(TEXT("NewKey")), FString(TEXT("NewValue")))
}

N_TEST_HIGH(FNWidgetStateTests_String_AddReturnsCorrectIndex,
    "NEXUS::UnitTests::NUI::FNWidgetState::String::AddReturnsCorrectIndex",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Verifies that AddString returns a 0-based index reflecting the insertion order.
    FNWidgetState State;
    const int32 FirstIndex = State.AddString(TEXT("First"), TEXT("A"));
    const int32 SecondIndex = State.AddString(TEXT("Second"), TEXT("B"));

    CHECK_EQUALS("First AddString should return index 0.", FirstIndex, 0)
    CHECK_EQUALS("Second AddString should return index 1.", SecondIndex, 1)
}

N_TEST_HIGH(FNWidgetStateTests_String_Remove,
    "NEXUS::UnitTests::NUI::FNWidgetState::String::Remove",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;
    State.AddString(TEXT("RemoveMe"), TEXT("gone"));
    State.RemoveString(TEXT("RemoveMe"));

    CHECK_FALSE_MESSAGE(TEXT("HasString should return false after RemoveString."), State.HasString(TEXT("RemoveMe")))
}

N_TEST_HIGH(FNWidgetStateTests_String_Clear,
    "NEXUS::UnitTests::NUI::FNWidgetState::String::Clear",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;
    State.AddString(TEXT("X"), TEXT("1"));
    State.AddString(TEXT("Y"), TEXT("2"));
    State.ClearStrings();

    CHECK_FALSE_MESSAGE(TEXT("HasString should return false after ClearStrings."), State.HasString(TEXT("X")))
    CHECK_FALSE_MESSAGE(TEXT("HasString should return false after ClearStrings."), State.HasString(TEXT("Y")))
}

N_TEST_HIGH(FNWidgetStateTests_ClearAll,
    "NEXUS::UnitTests::NUI::FNWidgetState::ClearAll",
    N_TEST_CONTEXT_ANYWHERE)
{
    FNWidgetState State;
    State.AddBoolean(TEXT("bFlag"), true);
    State.AddFloat(TEXT("Value"), 1.f);
    State.AddString(TEXT("Label"), TEXT("Test"));

    State.ClearAll();

    CHECK_FALSE_MESSAGE(TEXT("HasBoolean should return false after ClearAll."), State.HasBoolean(TEXT("bFlag")))
    CHECK_FALSE_MESSAGE(TEXT("HasFloat should return false after ClearAll."), State.HasFloat(TEXT("Value")))
    CHECK_FALSE_MESSAGE(TEXT("HasString should return false after ClearAll."), State.HasString(TEXT("Label")))
}

N_TEST_CRITICAL(FNWidgetStateTests_OverlayState_MergeNoReplace,
    "NEXUS::UnitTests::NUI::FNWidgetState::OverlayState::MergeNoReplace",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Verifies that OverlayState with bShouldReplaceKeys=false preserves existing keys and adds new ones.
    FNWidgetState Base;
    Base.AddBoolean(TEXT("bExisting"), false);
    Base.AddFloat(TEXT("Scale"), 1.f);
    Base.AddString(TEXT("Color"), TEXT("Red"));

    FNWidgetState Overlay;
    Overlay.AddBoolean(TEXT("bExisting"), true);   // conflict — should NOT replace
    Overlay.AddBoolean(TEXT("bNew"), true);         // new — should be added
    Overlay.AddFloat(TEXT("Scale"), 99.f);          // conflict — should NOT replace
    Overlay.AddFloat(TEXT("NewFloat"), 5.f);        // new — should be added
    Overlay.AddString(TEXT("Color"), TEXT("Blue")); // conflict — should NOT replace
    Overlay.AddString(TEXT("NewStr"), TEXT("Hi"));  // new — should be added

    Base.OverlayState(Overlay, false);

    CHECK_FALSE_MESSAGE(TEXT("Existing boolean should not be overwritten when bShouldReplaceKeys=false."), Base.GetBoolean(TEXT("bExisting")))
    CHECK_MESSAGE(TEXT("New boolean from overlay should be added."), Base.HasBoolean(TEXT("bNew")))
    CHECK_EQUALS("Existing float should not be overwritten when bShouldReplaceKeys=false.", Base.GetFloat(TEXT("Scale")), 1.f)
    CHECK_MESSAGE(TEXT("New float from overlay should be added."), Base.HasFloat(TEXT("NewFloat")))
    CHECK_EQUALS("Existing string should not be overwritten when bShouldReplaceKeys=false.", Base.GetString(TEXT("Color")), FString(TEXT("Red")))
    CHECK_MESSAGE(TEXT("New string from overlay should be added."), Base.HasString(TEXT("NewStr")))
}

N_TEST_CRITICAL(FNWidgetStateTests_OverlayState_Replace,
    "NEXUS::UnitTests::NUI::FNWidgetState::OverlayState::Replace",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Verifies that OverlayState with bShouldReplaceKeys=true overwrites conflicting keys from the other state.
    FNWidgetState Base;
    Base.AddBoolean(TEXT("bFlag"), false);
    Base.AddFloat(TEXT("Level"), 2.f);
    Base.AddString(TEXT("Name"), TEXT("Original"));

    FNWidgetState Overlay;
    Overlay.SetBoolean(TEXT("bFlag"), true);
    Overlay.SetFloat(TEXT("Level"), 10.f);
    Overlay.SetString(TEXT("Name"), TEXT("Replaced"));

    Base.OverlayState(Overlay, true);

    CHECK_MESSAGE(TEXT("Boolean should be replaced when bShouldReplaceKeys=true."), Base.GetBoolean(TEXT("bFlag")))
    CHECK_EQUALS("Float should be replaced when bShouldReplaceKeys=true.", Base.GetFloat(TEXT("Level")), 10.f)
    CHECK_EQUALS("String should be replaced when bShouldReplaceKeys=true.", Base.GetString(TEXT("Name")), FString(TEXT("Replaced")))
}

#endif //WITH_TESTS
