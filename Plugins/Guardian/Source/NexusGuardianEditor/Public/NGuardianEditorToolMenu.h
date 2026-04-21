// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Registers and drives the NexusGuardian entry in the editor's tool-menu, plus the
 * companion editor utility window.
 */
class FNGuardianEditorToolMenu
{
public:
	/** Register the tool-menu entries with UToolMenus. */
	static void Register();
	/** Unregister entries previously installed by Register. */
	static void Unregister();

	/** Open the Guardian editor utility widget tab, creating it if necessary. */
	static void CreateEditorUtilityWindow();
	/** @return true if the editor utility widget tab is currently open. */
	static bool HasEditorUtilityWindow();
};
