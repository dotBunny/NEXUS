// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

/**
 * Toggling a World Assembly tag across the current editor selection, on the actors in it or on the components.
 *
 * Shared rather than per-domain: the cell-ignore and world-collision-ignore commands do the same thing to a different
 * FName, and the tags themselves are declared together in NEXUS::WorldAssembly::ActorTags. What is domain-specific is
 * when each command is offered at all, and that stays on the command class as its CanExecute.
 *
 * @note Which of the two selections a command acts on is the command's decision, not this class's — only the
 *       world-collision tag means anything on a component, so only that command routes. See ToggleTagOnComponentSelection.
 * @see <a href="https://nexus-framework.com/docs/world-assembly/">World Assembly</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEditorTagUtils
{
public:
	/**
	 * Add Tag to every selected actor, or remove it from every selected actor when they all already carry it.
	 *
	 * @param Tag The actor tag to toggle.
	 * @param AddTransaction Undo description used when the call adds the tag.
	 * @param RemoveTransaction Undo description used when the call removes it.
	 * @note Adds only when no selected actor carries the tag. A partially-tagged selection removes, which is what
	 *       makes the tag reachable as a clear-all: the alternative would leave a mixed selection with no way back to
	 *       untagged in one click.
	 * @note No-ops on an empty selection.
	 */
	static void ToggleTagOnSelection(FName Tag, const FText& AddTransaction, const FText& RemoveTransaction);

	/**
	 * @param Tag The actor tag to test for.
	 * @return true if any selected actor already carries Tag.
	 * @note The branch ToggleTagOnSelection takes, exposed so a command can draw itself as the thing it will do.
	 *       "Any" rather than "all" for the reason given on ToggleTagOnSelection: a partially-tagged selection
	 *       removes, so it is also the answer to "will the next click remove".
	 */
	static bool IsTagOnAnySelectedActor(FName Tag);

	/**
	 * Add Tag to every selected component, or remove it from all of them when any already carries it.
	 *
	 * @param Tag The component tag to toggle.
	 * @param AddTransaction Undo description used when the call adds the tag.
	 * @param RemoveTransaction Undo description used when the call removes it.
	 * @note The component counterpart of ToggleTagOnSelection, on the same add/remove rule. It exists because the
	 *       actor is often not the unit an author can pick: a generator writes its whole result onto one container
	 *       actor, and tagging that actor takes all of it.
	 * @note No-ops on an empty component selection. Selecting a component in the level editor leaves its owning actor
	 *       selected too, so a command offering both must ask HasComponentsSelected first or it will only ever see
	 *       the actor.
	 */
	static void ToggleTagOnComponentSelection(FName Tag, const FText& AddTransaction, const FText& RemoveTransaction);

	/**
	 * @param Tag The component tag to test for.
	 * @return true if any selected component already carries Tag.
	 * @note "Any" rather than "all", matching IsTagOnAnySelectedActor — so it is also the answer to "will the next
	 *       click remove".
	 */
	static bool IsTagOnAnySelectedComponent(FName Tag);

	/** @return true when the editor has at least one component selected, which is what routes a command to the component path. */
	static bool HasComponentsSelected();
};
