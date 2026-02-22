// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NMenuEntry.h"

class FNToolsMenu
{
public:
	NEXUSTOOLINGEDITOR_API static void AddMenuEntry(const FNMenuEntry& Item);
	NEXUSTOOLINGEDITOR_API static void RemoveMenuEntry(FName Identifier);
	
	static void GenerateMenu(UToolMenu* Menu, bool bIsContextMenu);
private:
	
	static TMap<FName, FNMenuEntry> Entries;
	static TMap<FName, FText> Sections;
};

