// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

class AActor;

/**
 * Adds the selection-lock toggle to the actor context menu.
 *
 * Extends LevelEditor.ActorContextMenu, which the Outliner's own context menu is registered as a
 * derived menu of, so the entry appears both on an Outliner row and on a viewport right-click.
 * @see <a href="https://nexus-framework.com/docs/tooling/editor-types/selection-lock/selection-lock-menu/">FNSelectionLockMenu</a>
 */
class NEXUSTOOLINGEDITOR_API FNSelectionLockMenu
{
public:
	/** Add the NEXUS section and its toggle to the actor context menu. */
	static void AddMenuEntries();

	/** Remove the NEXUS section from the actor context menu. */
	static void RemoveMenuEntries();

private:
	/** Identifier of the section this feature owns within the actor context menu. */
	static FName SectionName;

	/** @return the actors currently selected in the level editor. */
	static TArray<AActor*> GetSelectedActors();

	/** Toggle the lock across the current selection. */
	static void ToggleSelectionLock();

	/** @return true when at least one selected actor can carry a lock. */
	static bool ToggleSelectionLock_CanExecute();

	/** @return true when every lockable actor in the selection is locked, which draws the check. */
	static bool ToggleSelectionLock_IsChecked();
};
