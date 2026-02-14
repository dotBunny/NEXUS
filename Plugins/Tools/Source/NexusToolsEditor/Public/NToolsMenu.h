// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NToolsMenuItem.h"

class FNToolsMenu
{
public:
	NEXUSTOOLSEDITOR_API static void AddCommand(const FNToolsMenuItem& Item);
	NEXUSTOOLSEDITOR_API static void RemoveCommand(FName Identifier);
	
	static void GenerateMenu(UToolMenu* Menu, bool bIsContextMenu);
private:
	
	static TMap<FName, FNToolsMenuItem> Items;
	static TMap<FName, FText> Sections;
};

