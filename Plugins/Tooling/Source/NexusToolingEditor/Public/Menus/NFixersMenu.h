// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NMenuEntry.h"

/**
 * Registry and generator for the "Fixers" dropdown menu. Plugins contribute entries via
 * AddMenuEntry during startup; the Register/Unregister pair wires the dropdown into the
 * global tool-menu system.
 */
class FNFixersMenu
{
public:

	/** Attach the Fixers dropdown to the tool-menu system. */
	static void Register();

	/** Detach the Fixers dropdown and release any tool-menu extensions it owned. */
	static void Unregister();

	/** Register a new entry in the Fixers menu; safe to call after Register(). */
	NEXUSTOOLINGEDITOR_API static void AddMenuEntry(const FNMenuEntry& Item);

	/** Remove a previously-registered Fixers entry by identifier. */
	NEXUSTOOLINGEDITOR_API static void RemoveMenuEntry(FName Identifier);

	/** Populate Menu with all registered entries, grouped by section. */
	static void GenerateMenu(UToolMenu* Menu, bool bIsContextMenu);
private:


	/** All registered menu entries keyed by their stable identifier. */
	static TMap<FName, FNMenuEntry> Entries;

	/** Display names for each section identifier referenced by Entries. */
	static TMap<FName, FText> Sections;
};
