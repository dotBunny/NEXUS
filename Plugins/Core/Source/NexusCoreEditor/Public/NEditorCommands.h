// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorStyle.h"

struct FNEditorCommandInfo
{
	FName Section = TEXT("General");
	FName Identifier;
	FText DisplayName;
	FText Tooltip;
	FExecuteAction Execute;
	FCanExecuteAction CanExecute;
	FIsActionChecked IsChecked;
	bool bIsMenuEntry = false;
	FNewToolMenuChoice MenuChoice;
	FSlateIcon Icon;
};

class FNEditorCommands final : public TCommands<FNEditorCommands>
{
public:
	FNEditorCommands()
		: TCommands<FNEditorCommands>(
			TEXT("NEditorCommands"),
			NSLOCTEXT("Contexts", "NEditorCommands", "NEditorCommands"),
			NAME_None,
			FNEditorStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;
	
	static void OnHelpOverwatch();
	static void OnHelpIssues();
	static void OnHelpBugReport();
	static void OnHelpDiscord();
	static void OnHelpRoadmap();
	static void OnHelpDocumentation();
	
	static void OnToolsProfileNetworkProfiler();
	static bool HasToolsProfileNetworkProfiler();
	
	static void OnWindowCleanLogsFolder();
	
	static void OnNodeExternalDocumentation();
	static bool NodeExternalDocumentation_CanExecute();

	static void BuildMenus();
	
	static void FillHelpSubMenu(UToolMenu* Menu);
	static void FillProjectLevelsSubMenu(UToolMenu* Menu);
	
	static void FillWindowMenu(UToolMenu* Menu, bool bIsContextMenu) { FillMenu(Menu, bIsContextMenu, WindowSections, WindowCommandInfo); }
	static void FillToolMenu(UToolMenu* Menu, bool bIsContextMenu) { FillMenu(Menu, bIsContextMenu, ToolsSections, ToolsCommandInfo); }
	
	NEXUSCOREEDITOR_API static void AddWindowCommand(FNEditorCommandInfo CommandInfo);
	NEXUSCOREEDITOR_API static void RemoveWindowCommand(FName Identifier);
	
	NEXUSCOREEDITOR_API static void AddToolCommand(FNEditorCommandInfo CommandInfo);
	NEXUSCOREEDITOR_API static void RemoveToolCommand(FName Identifier);

	NEXUSCOREEDITOR_API static void FillMenu(UToolMenu* Menu, bool bIsContextMenu, const TMap<FName, FText>& Sections, const TMap<FName, FNEditorCommandInfo>& Actions);
	
private:
	
	static TMap<FName, FNEditorCommandInfo> WindowCommandInfo;
	static TMap<FName, FText> WindowSections;
	
	static TMap<FName, FNEditorCommandInfo> ToolsCommandInfo;
	static TMap<FName, FText> ToolsSections;
	
	TSharedPtr<FUICommandList> CommandList_Help;
	TSharedPtr<FUICommandInfo> CommandInfo_Help_Discord;
	TSharedPtr<FUICommandInfo> CommandInfo_Help_BugReport;
	TSharedPtr<FUICommandInfo> CommandInfo_Help_Overwatch;
	TSharedPtr<FUICommandInfo> CommandInfo_Help_Roadmap;
	TSharedPtr<FUICommandInfo> CommandInfo_Help_Issues;
	TSharedPtr<FUICommandInfo> CommandInfo_Help_Documentation; 

	TSharedPtr<FUICommandList> CommandList_Node;
	TSharedPtr<FUICommandInfo> CommandInfo_Node_ExternalDocumentation;

	TSharedPtr<FUICommandList> CommandList_Tools;
	TSharedPtr<FUICommandInfo> CommandInfo_Tools_Profile_NetworkProfiler;
};