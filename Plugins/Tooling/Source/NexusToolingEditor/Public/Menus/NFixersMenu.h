// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NMenuEntry.h"

class FNFixersMenu
{
public:
	
	static void Register();
	static void Unregister();
	
	NEXUSTOOLINGEDITOR_API static void AddMenuEntry(const FNMenuEntry& Item);
	NEXUSTOOLINGEDITOR_API static void RemoveMenuEntry(FName Identifier);
	
	static void GenerateMenu(UToolMenu* Menu, bool bIsContextMenu);
private:
	
	
	static TMap<FName, FNMenuEntry> Entries;
	static TMap<FName, FText> Sections;
};
