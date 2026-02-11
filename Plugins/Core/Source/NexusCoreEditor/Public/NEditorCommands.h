// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorStyle.h"

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
	
	static void OnNodeExternalDocumentation();
	static bool NodeExternalDocumentation_CanExecute();

	static void BuildMenus();
	
	static void FillHelpSubMenu(UToolMenu* Menu);
	static void FillProjectLevelsSubMenu(UToolMenu* Menu);

private:
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