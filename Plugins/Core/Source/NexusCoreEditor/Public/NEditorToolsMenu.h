// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorCommandInfo.h"

class FNEditorToolsMenu
{
public:
	NEXUSCOREEDITOR_API static void AddCommand(FNEditorCommandInfo CommandInfo);
	NEXUSCOREEDITOR_API static void RemoveCommand(FName Identifier);
	
	static void RegisterCommands();
	static void GenerateMenu(UToolMenu* Menu, bool bIsContextMenu);
private:
	
	static void OnWindowCleanLogsFolder();
	
	static TMap<FName, FNEditorCommandInfo> CommandInfos;
	static TMap<FName, FText> Sections;
};

