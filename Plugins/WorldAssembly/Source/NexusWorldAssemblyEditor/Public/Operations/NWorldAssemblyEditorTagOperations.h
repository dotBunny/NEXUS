// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

/**
 * Toggling a World Assembly actor tag across the current editor selection.
 *
 * Shared rather than per-domain: the cell-ignore and world-collision-ignore commands do the same thing to a different
 * FName, and the tags themselves are declared together in NEXUS::WorldAssembly::ActorTags. What is domain-specific is
 * when each command is offered at all, and that stays on the command class as its CanExecute.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/">World Assembly</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEditorTagOperations
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
};
