// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EdMode/NWorldAssemblyRails.h"
#include "Widgets/SCompoundWidget.h"

/**
 * The World Assembly edit mode's category strip — World, Cell, Junction, Organ — pinned to the viewport.
 *
 * Buttons only. Selecting one writes to the shared FNWorldAssemblyRailState, which is what SNWorldAssemblyRailPanel
 * watches to decide what to show beside it. A category with nothing in the level to act on is hidden outright rather
 * than greyed, so the strip is only ever as tall as it has categories worth offering.
 *
 * @note Fixed in place, matching how Mesh Terrain mode pins its submode palette. The strip is the mode's anchor — it
 *       is what the user aims at to change category, so it staying put is worth more than being able to move it. The
 *       panel it drives is the draggable half.
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API SNWorldAssemblyRail final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNWorldAssemblyRail) {}

		/** Selection and availability, shared with the panel this strip drives. */
		SLATE_ARGUMENT(TSharedPtr<FNWorldAssemblyRails>, State)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	/**
	 * Build the vertical strip of category buttons.
	 *
	 * @return The category toolbar.
	 * @note Styled CategoryDrivenContentBuilderToolbarWithLabels on FVerticalToolBarBuilder, which is exactly what
	 *       FCategoryDrivenContentBuilderBase::GetCategoryToolBarStyleName resolves to for a labelled rail. Borrowed
	 *       rather than reinvented so the strip reads as a stock editor mode rail, which is what it is.
	 */
	TSharedRef<SWidget> CreateCategoryToolBar();

	/**
	 * Re-poll category availability.
	 *
	 * @return Always Continue; the strip watches for as long as it exists.
	 * @note Lives here rather than on the panel because the strip is the half that is always on screen, and because
	 *       the buttons it draws are most of what the poll's result is for.
	 */
	EActiveTimerReturnType TickAvailability(double InCurrentTime, float InDeltaTime);

	/** Selection and availability, shared with the panel. */
	TSharedPtr<FNWorldAssemblyRails> State;

	/**
	 * Command list carrying just the category actions.
	 * @note Its own list rather than the toolkit's. These actions carry visibility predicates, and a command list
	 *       resolves an action by command, so keeping them here means those predicates cannot reach a tool command
	 *       mapped into the toolkit list. The buttons each category builds resolve against that list instead, which
	 *       the rails already hold.
	 */
	TSharedPtr<FUICommandList> CategoryCommandList;
};
