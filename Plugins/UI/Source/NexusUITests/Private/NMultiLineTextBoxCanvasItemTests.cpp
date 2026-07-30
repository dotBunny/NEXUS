// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "CanvasItems/NMultiLineTextBoxCanvasItem.h"
#include "Tests/TestHarnessAdapter.h"
#include "Macros/NTestMacros.h"

// These tests cover the observable public state machine of FNMultiLineTextBoxCanvasItem. The measured layout
// (Process and the per-line Width/Height/Position) is private and only exercised through FNCanvasUtils::DrawCanvasTextBox
// with a live FCanvas, so it is validated by the editor draw path rather than here. What matters for correctness is the
// invariant the draw path relies on: every line mutation marks the item dirty (forcing a re-layout before the next draw),
// and Clear fully resets the item.

N_TEST_HIGH(FNMultiLineTextBoxCanvasItemTests_Content_EmptyByDefault,
    "NEXUS::UnitTests::NUI::FNMultiLineTextBoxCanvasItem::Content::EmptyByDefault",
    N_TEST_CONTEXT_ANYWHERE)
{
    const FNMultiLineTextBoxCanvasItem TextBox;

    CHECK_FALSE_MESSAGE(TEXT("A freshly constructed item should report no content."), TextBox.HasContent())
    CHECK_FALSE_MESSAGE(TEXT("A freshly constructed item should not be dirty."), TextBox.IsDirty())
}

N_TEST_CRITICAL(FNMultiLineTextBoxCanvasItemTests_Content_AddSmallLineMarksDirty,
    "NEXUS::UnitTests::NUI::FNMultiLineTextBoxCanvasItem::Content::AddSmallLineMarksDirty",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Verifies AddSmallLine both registers content and marks the item dirty so the next draw re-runs layout.
    FNMultiLineTextBoxCanvasItem TextBox;
    TextBox.AddSmallLine(FText::FromString(TEXT("Hello")));

    CHECK_MESSAGE(TEXT("AddSmallLine should register content."), TextBox.HasContent())
    CHECK_MESSAGE(TEXT("AddSmallLine should mark the item dirty."), TextBox.IsDirty())
}

N_TEST_CRITICAL(FNMultiLineTextBoxCanvasItemTests_Content_AddLargeLineMarksDirty,
    "NEXUS::UnitTests::NUI::FNMultiLineTextBoxCanvasItem::Content::AddLargeLineMarksDirty",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Verifies AddLargeLine both registers content and marks the item dirty so the next draw re-runs layout.
    FNMultiLineTextBoxCanvasItem TextBox;
    TextBox.AddLargeLine(FText::FromString(TEXT("Header")));

    CHECK_MESSAGE(TEXT("AddLargeLine should register content."), TextBox.HasContent())
    CHECK_MESSAGE(TEXT("AddLargeLine should mark the item dirty."), TextBox.IsDirty())
}

N_TEST_CRITICAL(FNMultiLineTextBoxCanvasItemTests_Content_ClearResetsState,
    "NEXUS::UnitTests::NUI::FNMultiLineTextBoxCanvasItem::Content::ClearResetsState",
    N_TEST_CONTEXT_ANYWHERE)
{
    // Verifies Clear empties content and resets the dirty flag after several lines have been added.
    FNMultiLineTextBoxCanvasItem TextBox;
    TextBox.AddSmallLine(FText::FromString(TEXT("Line 1")));
    TextBox.AddLargeLine(FText::FromString(TEXT("Line 2")));
    TextBox.Clear();

    CHECK_FALSE_MESSAGE(TEXT("Clear should remove all content."), TextBox.HasContent())
    CHECK_FALSE_MESSAGE(TEXT("Clear should reset the dirty flag."), TextBox.IsDirty())
}

#endif //WITH_TESTS
