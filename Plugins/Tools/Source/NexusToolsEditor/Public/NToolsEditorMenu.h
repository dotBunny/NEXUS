// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorCommandInfo.h"

class FNToolsEditorMenu
{
public:
	NEXUSTOOLSEDITOR_API static void AddCommand(FNEditorCommandInfo CommandInfo);
	NEXUSTOOLSEDITOR_API static void RemoveCommand(FName Identifier);
	
	static void GenerateMenu(UToolMenu* Menu, bool bIsContextMenu);
private:
	
	static TMap<FName, FNEditorCommandInfo> CommandInfos;
	static TMap<FName, FText> Sections;
};

