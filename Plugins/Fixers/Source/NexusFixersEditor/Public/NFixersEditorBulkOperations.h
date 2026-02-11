// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorCommandInfo.h"

class FNFixersEditorBulkOperations
{
public:
	
	static void Register();
	static void Unregister();
	
	NEXUSFIXERSEDITOR_API static void AddCommand(const FNEditorCommandInfo& Command);
	NEXUSFIXERSEDITOR_API static void RemoveCommand(FName Identifier);
	
private:
	static void FillContextBulkOperationsMenu(UToolMenu* Menu, bool bIsContextMenu);
	
	static TMap<FName, FNEditorCommandInfo> CommandInfo;
	static TMap<FName, FText> Sections;
};
