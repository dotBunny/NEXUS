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
	
	TSharedPtr<FUICommandList> CommandList_Help;
	TSharedPtr<FUICommandInfo> CommandInfo_Help_BugReport;
	TSharedPtr<FUICommandInfo> CommandInfo_Help_Overwatch;
	TSharedPtr<FUICommandInfo> CommandInfo_Help_Roadmap;
	TSharedPtr<FUICommandInfo> CommandInfo_Help_Issues;
	TSharedPtr<FUICommandInfo> CommandInfo_Help_Documentation; 

	TSharedPtr<FUICommandList> CommandList_Node;
	TSharedPtr<FUICommandInfo> CommandInfo_Node_ExternalDocumentation;

	TSharedPtr<FUICommandList> CommandList_Tools;
	TSharedPtr<FUICommandInfo> CommandInfo_Tools_LeakCheck; 

	static void OnHelpOverwatch();
	static void OnHelpIssues();
	static void OnHelpBugReport();
	static void OnHelpRoadmap();
	static void OnHelpDocumentation();
	
	static void OnToolsLeakCheck();
	static bool OnToolsLeakCheck_CanExecute();

	static void OnNodeExternalDocumentation();
	static bool OnNodeExternalDocumentation_CanExecute();

	static void BuildMenus();
	static void FillHelpSubMenu(UToolMenu* Menu);
	static void FillProjectLevelsSubMenu(UToolMenu* Menu);
};
