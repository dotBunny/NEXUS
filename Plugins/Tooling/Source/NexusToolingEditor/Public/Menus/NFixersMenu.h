// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NMenuEntry.h"

/**
 * Registry and generator for the "Fixers" dropdown menu. Plugins contribute entries via
 * AddMenuEntry during startup; the dropdown itself is wired into the tool-menu system by
 * FNToolingEditorCommands::AddMenuEntries, which binds GenerateMenu as its content delegate.
 */
class FNFixersMenu
{
public:

	/** Register a new entry in the Fixers menu. */
	NEXUSTOOLINGEDITOR_API static void AddMenuEntry(const FNMenuEntry& Item);

	/** Remove a previously-registered Fixers entry by identifier. */
	NEXUSTOOLINGEDITOR_API static void RemoveMenuEntry(FName Identifier);

	/** Populate Menu with all registered entries, grouped by section. */
	static void GenerateMenu(UToolMenu* Menu, bool bIsContextMenu);
private:


	/** All registered menu entries keyed by their stable identifier. */
	static TMap<FName, FNMenuEntry> Entries;
};
