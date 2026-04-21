// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Editor command entry points registered at module startup, used to add/remove Actor Pool menu items
 * and inject helper methods when specific assets are being edited.
 */
class FNActorPoolsEditorCommands
{
public:
	/** Register Actor Pool menu entries with the editor. */
	static void AddMenuEntries();
	/** Unregister the menu entries previously added by AddMenuEntries. */
	static void RemoveMenuEntries();

	/**
	 * Inject Actor Pool helper methods into an asset editor when editing a relevant object type.
	 * @param EditedObject The object currently being edited in the asset editor.
	 */
	static void AddActorPoolMethods(const UObject* EditedObject);
};
