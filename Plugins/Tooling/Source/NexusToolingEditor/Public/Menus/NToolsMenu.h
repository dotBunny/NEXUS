// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NMenuEntry.h"

/**
 * Registry and generator for the NEXUS "Tools" dropdown menu. Other plugins contribute their
 * tool-menu entries here via AddMenuEntry so everything appears under a single top-level menu.
 */
class FNToolsMenu
{
public:
	/** Register a new entry in the Tools menu. */
	NEXUSTOOLINGEDITOR_API static void AddMenuEntry(const FNMenuEntry& Item);

	/** Remove a previously-registered Tools entry by identifier. */
	NEXUSTOOLINGEDITOR_API static void RemoveMenuEntry(FName Identifier);

	/** Populate Menu with all registered entries, grouped by section. */
	static void GenerateMenu(UToolMenu* Menu, bool bIsContextMenu);
private:

	/** All registered menu entries keyed by their stable identifier. */
	static TMap<FName, FNMenuEntry> Entries;

	/** Display names for each section identifier referenced by Entries. */
	static TMap<FName, FText> Sections;
};

