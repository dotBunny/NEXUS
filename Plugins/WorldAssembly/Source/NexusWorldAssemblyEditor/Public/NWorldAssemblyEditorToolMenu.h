// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Organ/NOrganComponent.h"

/**
 * Registers every World Assembly entry on the editor's tool menus — cell/organ/junction dropdowns,
 * toolbar buttons, and the editor-utility-window launcher.
 *
 * @note Menu construction only. The commands these entries fire, and the Quick Assembly state the widgets below
 *       read, live on the per-category command classes — this depends on those, never the other way round.
 */
class FNWorldAssemblyEditorToolMenu
{
public:

	/** Name of the global tool-menu section World Assembly entries are added to. */
	static const FName MenuSectionGlobal;

	/** Register all World Assembly entries with the global tool-menu system. */
	static void AddMenuEntries();

	/** Remove every entry previously registered by AddMenuEntries. */
	static void RemoveMenuEntries();

	/** @return true if the organ dropdown should be visible given the current context. */
	static bool ShowOrganDropdown();

	/**
	 * @return The toolbar's organ picker, showing the Quick Assembly target and its progress bar.
	 * @note Opens an empty menu while FNWorldAssemblyEditorQuickAssemblyCommands::IsActive, which is the visible half
	 *       of the selection lock; SetSelectedOrgan enforces it regardless.
	 */
	static TSharedRef<SWidget> CreateQuickAssemblyComboBox();

	/** @return A numeric-entry widget bound to UNWorldAssemblyEditorUserSettings::QuickAssemblyAutoAssemblyTimer (seconds), for the Quick Assembly options dropdown. */
	static TSharedRef<SWidget> CreateQuickAssemblyAutoAssemblyTimerWidget();
};
