// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EdMode/NWorldAssemblyRails.h"
#include "Widgets/SCompoundWidget.h"

class SWidgetSwitcher;

/**
 * The content of whichever category SNWorldAssemblyRail has selected: its picker and its buttons, over the focused
 * cell's warnings.
 *
 * The draggable half of the edit mode's UI. Every category's content is built once and swapped between with an
 * SWidgetSwitcher, so the pickers keep their state across a switch and only the shown category is laid out.
 *
 * @note Deliberately untitled. Each category's own header names what it acts on, and the strip's lit icon says which
 *       category is showing — a heading repeating the category name on top of both was noise.
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API SNWorldAssemblyRailPanel final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNWorldAssemblyRailPanel) {}

		/** Selection and availability, shared with the strip driving this panel. */
		SLATE_ARGUMENT(TSharedPtr<FNWorldAssemblyRails>, State)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	/**
	 * Build one category's content: its picker above its buttons.
	 *
	 * @param Rail The category to build for.
	 * @return That category's content, scrolled.
	 * @note The picker names what the category acts on, so it sits above the buttons that act on it.
	 * @note Scrolled because Cell alone carries six titled groups, which outgrows anything that reads as an overlay
	 *       rather than a panel.
	 */
	static TSharedRef<SWidget> CreateCategoryContent(const TSharedRef<FNEdModeRail>& Rail);

	/**
	 * @return The focused cell's warnings, boxed, collapsing to nothing when there are none.
	 * @note Pinned below the content rather than scrolling with it: this reports live cell state, so scrolling it out
	 *       of reach defeats it. Amber rather than red — everything GetWarningText reports is a "this will not
	 *       regenerate on save" advisory, not a failure.
	 */
	static TSharedRef<SWidget> CreateWarningFooter();

	/** Selection and availability, shared with the strip. */
	TSharedPtr<FNWorldAssemblyRails> State;

	/** Switches to the selected category's content; indices line up with the state's rails. */
	TSharedPtr<SWidgetSwitcher> ContentSwitcher;
};
