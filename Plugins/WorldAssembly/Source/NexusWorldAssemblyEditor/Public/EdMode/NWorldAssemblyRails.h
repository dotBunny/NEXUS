// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NEdModeRail.h"

/**
 * Which rail category is selected, and which are worth showing at all.
 *
 * Held by shared pointer and read by both halves of the edit mode's UI: SNWorldAssemblyRail draws the category strip
 * and writes the selection, SNWorldAssemblyRailPanel reads it to decide what to show. Neither widget owns the other,
 * and they live in separate viewport overlays, so the state they agree on cannot live in either of them.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyRails
{
public:
	/** @param InRails The categories, in the order their buttons appear down the rail. */
	explicit FNWorldAssemblyRails(TArray<TSharedRef<FNEdModeRail>> InRails);

	/** @return The categories, in rail order. */
	const TArray<TSharedRef<FNEdModeRail>>& GetRails() const { return Rails; }

	/**
	 * @return Position in GetRails of the selected category, or INDEX_NONE for no category at all — which the user can
	 *         ask for by clicking the selected category's button, and which the panel reads as closed.
	 */
	int32 GetActiveIndex() const { return ActiveIndex; }

	/**
	 * Select a category, ending whatever tool the outgoing one left running.
	 *
	 * @param Index Position in GetRails, or INDEX_NONE to select nothing and close the panel.
	 * @note Leaving a category leaves its tool. The tools are scoped to their category — the hull tools belong to
	 *       Cell, junction placement to Junction — so one left running under a category that is no longer presented
	 *       keeps drawing handles and swallowing viewport clicks with nothing on screen to say what is doing it or how
	 *       to stop it. Safe to call from the availability poll: UEditorInteractiveToolsContext defers the shutdown to
	 *       its next tick rather than tearing the tool down mid-frame.
	 */
	void SetActiveIndex(int32 Index);

	/**
	 * Select a category, or close the panel when it is the one already selected.
	 *
	 * @param Index Position in GetRails.
	 * @note What a rail button does, as opposed to what selecting a category does — the strip is the only way to the
	 *       closed state, and a lit button that does nothing when clicked is the one thing a toggle must not be.
	 */
	void ToggleActiveIndex(int32 Index);

	/**
	 * @param Index Position in GetRails.
	 * @return true when that category has something in the level to act on.
	 * @note Reads the cache RefreshAvailability fills, and must stay cheap enough to call every frame: this backs the
	 *       category buttons' visibility delegates, which the multibox evaluates on every paint.
	 */
	bool IsAvailable(int32 Index) const;

	/**
	 * Re-evaluate every category's availability predicate into the cache, and move off the active category if it is
	 * the one that just became unavailable.
	 *
	 * @note Polled rather than read straight from the predicates, because they are not cheap: Cell's and Junction's
	 *       walk every level's actor list looking for an ANCellActor. Bound directly to the buttons' visibility
	 *       delegates that would run twice per frame.
	 * @note The fallback is the half hiding categories needs that greying them out did not. A disabled button that is
	 *       already selected simply stays selected, but a hidden one would leave the strip pointing at a category with
	 *       no button while the panel beside it still showed that category's content.
	 * @note Nothing to fall back from where the user has closed the panel: INDEX_NONE is a state they asked for rather
	 *       than one a vanished category left behind, and recovering from it would reopen the panel a quarter of a
	 *       second after it was shut.
	 */
	void RefreshAvailability();

private:
	/**
	 * Choose the category to open on: the first available one the level's contents point at, or failing that the first
	 * available one outright.
	 *
	 * @note Runs once, from the constructor — which is once per mode entry, since the toolkit holding this is rebuilt
	 *       every time the mode opens. Nothing re-seeds afterwards: past that first frame the selection is the user's,
	 *       and the only thing that moves it is RefreshAvailability's fallback taking it off a category that has gone.
	 * @note First available rather than the first outright: Cell and Junction both sit above nothing when the level has
	 *       no cell actor, and opening on a category whose button is hidden would show content with no way back to it.
	 * @note Rail order is the precedence, so a level somehow holding both a cell and organs opens on Cell. The two are
	 *       mutually exclusive by rule — see FNWorldEdModeRail's add commands — so this only decides for
	 *       data authored before that rule existed.
	 */
	void SeedActiveIndex();

	/** The categories, in rail order. */
	TArray<TSharedRef<FNEdModeRail>> Rails;

	/** Last polled availability per category; indices line up with Rails. */
	TArray<bool> Available;

	/** Position in Rails of the selected category. */
	int32 ActiveIndex = INDEX_NONE;
};
