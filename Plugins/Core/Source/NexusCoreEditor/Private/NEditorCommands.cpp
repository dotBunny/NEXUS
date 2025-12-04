// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorCommands.h"

#include "BlueprintEditor.h"
#include "NEditorSettings.h"
#include "NEditorUtils.h"
#include "NMetaUtils.h"
#include "DelayedEditorTasks/NLeakTestDelayedEditorTask.h"

#define LOCTEXT_NAMESPACE "NexusEditor"

TMap<FName, FNWindowCommandInfo> FNEditorCommands::WindowActions;

void FNEditorCommands::RegisterCommands()
{
	// ReSharper disable StringLiteralTypo
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Help_Overwatch,"NCore.Help.OpenOverwatch",
	LOCTEXT("Command_Help_OpenOverwatch", "Overwatch"),
	LOCTEXT("Command_Help_Overwatch_Desc", "Opens the GitHub project's development board in your browser."),
	FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.VisitCommunityHome"),
	EUserInterfaceActionType::Button, FInputChord());
	// ReSharper restore StringLiteralTypo
	

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Help_Issues,
	"NCore.Help.OpenIssues",
	LOCTEXT("Command_Help_OpenBugReport", "Issues"),
	LOCTEXT("Command_Help_OpenBugReport_Desc", "Opens the GitHub project's issue list in your browser."),
	FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.OpenIssueTracker"),
	EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Help_Discord,
	"NCore.Help.OpenDiscord",
	LOCTEXT("Command_Help_OpenDiscord", "Discord"),
	LOCTEXT("Command_Help_OpenDiscord_Desc", "Opens the Discord (dotBunny Support) invite link in your browser."),
	FSlateIcon(FNEditorStyle::GetStyleSetName(), "Command.OpenDiscordInviteLink"),
	EUserInterfaceActionType::Button, FInputChord());
	
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Help_BugReport,
	"NCore.Help.OpenBugReport",
	LOCTEXT("Command_Help_OpenBugReport", "Report a Bug"),
	LOCTEXT("Command_Help_OpenBugReport_Desc", "Opens the GitHub repository's bug report form in your browser."),
	FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.ReportABug"),
	EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Help_Roadmap,
		"NCore.Help.OpenRoadmap",
		LOCTEXT("Command_Help_OpenBugReport", "Roadmap"),
		LOCTEXT("Command_Help_OpenBugReport_Desc", "Opens the GitHub project's Roadmap in your browser."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.VisitOnlineLearning"),
		EUserInterfaceActionType::Button, FInputChord());	

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Help_Documentation,
		"NCore.Help.OpenDocumentation",
		LOCTEXT("Command_Help_OpenDocumentation", "Documentation"),
		LOCTEXT("Command_Help_OpenDocumentation_Desc", "Open the documentation in your browser."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Documentation"),
		EUserInterfaceActionType::Button, FInputChord());


	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Tools_LeakCheck,
	"NCore.Tools.LeakCheck",
	LOCTEXT("Command_Tools_LeakCheck", "Leak Check"),
	LOCTEXT("Command_Tools_LeakCheck_Desc", "Capture and process all UObjects over a period of 5 seconds to check for leaks."),
	FSlateIcon(FNEditorStyle::GetStyleSetName(), "Command.LeakCheck"),
	EUserInterfaceActionType::Button, FInputChord());


	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Tools_Profile_NetworkProfiler,
	"NCore.Tools.Profile.NetworkProfiler",
	LOCTEXT("Command_Tools_Profile_NetworkProfiler", "Network Profiler"),
	LOCTEXT("Command_Tools_Profile_NetworkProfiler", "Launch external NetworkProfiler tool."),
	FSlateIcon(FNEditorStyle::GetStyleSetName(), "Command.Visualizer"),
	EUserInterfaceActionType::Button, FInputChord());
	
	
	CommandList_Help = MakeShareable(new FUICommandList);
	
	CommandList_Help->MapAction(Get().CommandInfo_Help_Discord,
	FExecuteAction::CreateStatic(&FNEditorCommands::OnHelpDiscord),
	FCanExecuteAction());
	
	CommandList_Help->MapAction(Get().CommandInfo_Help_BugReport,
		FExecuteAction::CreateStatic(&FNEditorCommands::OnHelpBugReport),
		FCanExecuteAction());

	CommandList_Help->MapAction(Get().CommandInfo_Help_Overwatch,
		FExecuteAction::CreateStatic(&FNEditorCommands::OnHelpOverwatch),
		FCanExecuteAction());

	CommandList_Help->MapAction(Get().CommandInfo_Help_Issues,
	FExecuteAction::CreateStatic(&FNEditorCommands::OnHelpIssues),
	FCanExecuteAction());

	CommandList_Help->MapAction(Get().CommandInfo_Help_Roadmap,
		FExecuteAction::CreateStatic(&FNEditorCommands::OnHelpRoadmap),
		FCanExecuteAction());

	CommandList_Help->MapAction(Get().CommandInfo_Help_Documentation,
		FExecuteAction::CreateStatic(&FNEditorCommands::OnHelpDocumentation),
		FCanExecuteAction());


	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Node_ExternalDocumentation,
			"NCore.Node.ExternalDocumentation",
			LOCTEXT("Command_Node_OpenExternalDocumentation", "External Documentation"),
			LOCTEXT("Command_Help_OpenRepository_Desc", "Open the external documentation (DocsURL) about this function."),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Documentation"),
			EUserInterfaceActionType::Button, FInputChord());

	CommandList_Node = MakeShareable(new FUICommandList);

	CommandList_Node->MapAction(Get().CommandInfo_Node_ExternalDocumentation,
		FExecuteAction::CreateStatic(&FNEditorCommands::OnNodeExternalDocumentation),
		FCanExecuteAction::CreateStatic(&FNEditorCommands::NodeExternalDocumentation_CanExecute));


	CommandList_Tools = MakeShareable(new FUICommandList);
	CommandList_Tools->MapAction(Get().CommandInfo_Tools_LeakCheck,
		FExecuteAction::CreateStatic(&FNEditorCommands::OnToolsLeakCheck),
		FCanExecuteAction::CreateStatic(&FNEditorCommands::ToolsLeakCheck_CanExecute));
	CommandList_Tools->MapAction(Get().CommandInfo_Tools_Profile_NetworkProfiler,
		FExecuteAction::CreateStatic(&FNEditorCommands::OnToolsProfileNetworkProfiler));
}

// ReSharper disable once IdentifierTypo
void FNEditorCommands::OnHelpOverwatch()
{
	FPlatformProcess::LaunchURL(TEXT("https://github.com/orgs/dotBunny/projects/6/views/1"),nullptr, nullptr);
}

void FNEditorCommands::OnHelpIssues()
{
	FPlatformProcess::LaunchURL(TEXT("https://github.com/orgs/dotBunny/projects/6/views/3"),nullptr, nullptr);
}

void FNEditorCommands::OnHelpBugReport()
{
	FPlatformProcess::LaunchURL(TEXT("https://github.com/dotBunny/NEXUS/issues/new/choose"),nullptr, nullptr);
}

void FNEditorCommands::OnHelpDiscord()
{
	FPlatformProcess::LaunchURL(TEXT("https://discord.gg/2M9HczHanW"),nullptr, nullptr);
}

void FNEditorCommands::OnHelpRoadmap()
{
	FPlatformProcess::LaunchURL(TEXT("https://github.com/orgs/dotBunny/projects/6/views/2"),nullptr, nullptr);
}

void FNEditorCommands::OnHelpDocumentation()
{
	FPlatformProcess::LaunchURL(TEXT("https://nexus-framework.com/docs/"),nullptr, nullptr);
}

void FNEditorCommands::OnToolsLeakCheck()
{
	UNLeakTestDelayedEditorTask::Create();
}

bool FNEditorCommands::ToolsLeakCheck_CanExecute()
{
	return true;
}

void FNEditorCommands::OnToolsProfileNetworkProfiler()
{
	const FString ExecutablePath = FPaths::Combine(FNEditorUtils::GetEngineBinariesPath(), "DotNet", "NetworkProfiler.exe");
	constexpr bool bLaunchDetached = true;
	constexpr bool bLaunchHidden = false;
	constexpr bool bLaunchReallyHidden = false;
	const FProcHandle ProcHandle = FPlatformProcess::CreateProc(*ExecutablePath, TEXT(""), bLaunchDetached,
		bLaunchHidden, bLaunchReallyHidden, nullptr, 0, nullptr, nullptr, nullptr);
	if (!ProcHandle.IsValid())
	{
		UE_LOG(LogNexusCoreEditor, Error, TEXT("Unable to launch NetworkProfiler."))
	}
}

bool FNEditorCommands::HasToolsProfileNetworkProfiler()
{
	return FPaths::FileExists(FPaths::Combine(FNEditorUtils::GetEngineBinariesPath(), "DotNet", "NetworkProfiler.exe"));
}

void FNEditorCommands::OnNodeExternalDocumentation()
{
	FBlueprintEditor* Editor = FNEditorUtils::GetForegroundBlueprintEditor();
	if (Editor == nullptr) return;
	UEdGraphNode* Node = Editor->GetSingleSelectedNode();
	if (Node == nullptr) return;

	// Split the data so you can have multiple URIs in the data
	TArray<FString> OutURIs;
	FNMetaUtils::GetExternalDocumentation(Node).ParseIntoArray(OutURIs, TEXT(","), true);
	for (int i = 0; i < OutURIs.Num(); i++)
	{
		FPlatformProcess::LaunchURL(*OutURIs[i].TrimStartAndEnd(),nullptr, nullptr);
	}
}

bool FNEditorCommands::NodeExternalDocumentation_CanExecute()
{
	FBlueprintEditor* Editor = FNEditorUtils::GetForegroundBlueprintEditor();
	if (Editor == nullptr) return false;
	UEdGraphNode* Node = Editor->GetSingleSelectedNode();
	if (Node == nullptr) return false;
	return FNMetaUtils::HasExternalDocumentation(Node);
}

void FNEditorCommands::BuildMenus()
{
	const FNEditorCommands Commands = Get();

	// Project Levels
	if (UToolMenu* FileMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.File"))
	{
		FToolMenuSection& FileOpenSection = FileMenu->FindOrAddSection("FileOpen");
		FileOpenSection.AddSubMenu(
				"NProjectLevels",
				LOCTEXT("ProjectLevels", "Project Levels"),
				LOCTEXT("ProjectLevels_Tooltip", "A pre-defined list of levels related to the project."),
				FNewToolMenuDelegate::CreateStatic(&FillProjectLevelsSubMenu),
				false,
				FSlateIcon(FNEditorStyle::GetStyleSetName(), "Command.ProjectLevels")
			);
	}
	
	UToolMenu* WindowsMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
	FToolMenuSection& LevelEditorSection = WindowsMenu->FindOrAddSection("LevelEditor");
	LevelEditorSection.AddSubMenu(
			"NEXUS",
			LOCTEXT("NWindows", "NEXUS"),
			LOCTEXT("NWindows_ToolTip", "EUW/Windows added by parts of NEXUS."),
			FNewToolMenuDelegate::CreateStatic(&FillNexusWindowsMenu, true),
			false,
			FSlateIcon(FNEditorStyle::GetStyleSetName(), "NEXUS.Icon")
		);
	
	// Tools Menu
	if (UToolMenu* ToolMenus = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools"))
	{
		FToolMenuSection& ToolsSection = ToolMenus->FindOrAddSection("NEXUS");
		ToolsSection.Label = LOCTEXT("NLevelEditorTools", "NEXUS");

		ToolsSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Tools_LeakCheck, Commands.CommandList_Tools);
	}

	
	// Add in NetworkProfiler menu option if its present
	if (HasToolsProfileNetworkProfiler())
	{
		if (UToolMenu* ProfileMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools.Profile"))
		{
			FToolMenuSection& ExternalMenu = ProfileMenu->FindOrAddSection("External");
			ExternalMenu.Label = LOCTEXT("NLevelEditorToolsExternal", "External");

			ExternalMenu.AddMenuEntryWithCommandList(Commands.CommandInfo_Tools_Profile_NetworkProfiler, Commands.CommandList_Tools);
		}
	}
	
	// Help Menu Submenu
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Help"))
	{
		FToolMenuSection& ApplicationSection = Menu->FindOrAddSection(FName("Reference"));
		ApplicationSection.AddSubMenu(
				"NEXUS",
				LOCTEXT("NHelp", "NEXUS"),
				LOCTEXT("NHelp_Tooltip", "Help related to NEXUS"),
				FNewToolMenuDelegate::CreateStatic(&FillHelpSubMenu),
				false,
				FSlateIcon(FNEditorStyle::GetStyleSetName(), "NEXUS.Icon")
			);
	}
	
	// Support for DocsURL addition to nodes
	if (UToolMenu* BlueprintNodeContextMenu = UToolMenus::Get()->ExtendMenu("GraphEditor.GraphNodeContextMenu.K2Node_CallFunction"))
	{
		FToolMenuSection& DocumentationSection = BlueprintNodeContextMenu->FindOrAddSection(FName("EdGraphSchemaDocumentation"));
		DocumentationSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Node_ExternalDocumentation, Commands.CommandList_Node);
	}
}

void FNEditorCommands::FillProjectLevelsSubMenu(UToolMenu* Menu)
{
	FToolMenuSection& ProjectLevelsSection = Menu->AddSection("ProjectLevels", LOCTEXT("ProjectLevels", ""));
	for (const UNEditorSettings* Settings = UNEditorSettings::Get();
		const FSoftObjectPath& Path : Settings->ProjectLevels)
	{
		if (!Path.IsValid())
		{
			continue;
		}
		
		const FText DisplayName = FText::FromString(Path.GetAssetName());
		const FText DisplayTooltip = FText::FromString(Path.GetAssetPathString());
		const FName AssetName = Path.GetAssetFName();

		FUIAction ButtonAction = FUIAction(
		FExecuteAction::CreateLambda([Path]()
			{
				if (const FString MapPath = Path.ToString(); MapPath.Len() > 0)
				{
					GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(MapPath);
				}
			}),
			FCanExecuteAction::CreateStatic(&FNEditorUtils::IsNotPlayInEditor),
			FIsActionChecked(),
			FIsActionButtonVisible());

		ProjectLevelsSection.AddMenuEntry(Path.GetAssetFName(), DisplayName, DisplayTooltip,
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Level"),
			FToolUIActionChoice(ButtonAction),
			EUserInterfaceActionType::Button, AssetName);
	}
}

void FNEditorCommands::FillNexusWindowsMenu(UToolMenu* Menu, bool bIsContextMenu)
{
	FToolMenuSection& WindowsSection = Menu->AddSection("Windows", LOCTEXT("Windows", ""));
	for (auto WindowCommand : WindowActions)
	{
		FUIAction ButtonAction = FUIAction(WindowCommand.Value.Execute,WindowCommand.Value.CanExecute, 
			WindowCommand.Value.IsChecked, FIsActionButtonVisible());
		
		if (WindowCommand.Value.IsChecked.IsBound())
		{
			WindowsSection.AddMenuEntry(WindowCommand.Value.Identifier,  WindowCommand.Value.DisplayName, 
			WindowCommand.Value.Tooltip, WindowCommand.Value.Icon,
			FToolUIActionChoice(ButtonAction), EUserInterfaceActionType::Check);
		}
		else
		{
			WindowsSection.AddMenuEntry(WindowCommand.Value.Identifier,  WindowCommand.Value.DisplayName, 
			WindowCommand.Value.Tooltip, WindowCommand.Value.Icon,
			FToolUIActionChoice(ButtonAction), EUserInterfaceActionType::Button);
		}
	}
}

void FNEditorCommands::AddWindowCommand(FNWindowCommandInfo CommandInfo)
{
	if (!WindowActions.Contains(CommandInfo.Identifier))
	{
		WindowActions.Add(CommandInfo.Identifier, CommandInfo);
	}
	else
	{
		WindowActions[CommandInfo.Identifier] = CommandInfo;
	}
}

void FNEditorCommands::RemoveWindowCommand(const FName Identifier)
{
	if (WindowActions.Contains(Identifier))
	{
		WindowActions.Remove(Identifier);
	}
}

void FNEditorCommands::FillHelpSubMenu(UToolMenu* Menu)
{
	const FNEditorCommands Commands = FNEditorCommands::Get();

	FToolMenuSection& ReferenceSection = Menu->FindOrAddSection("Reference");
	ReferenceSection.Label = LOCTEXT("NHelp_Reference", "Reference");
	ReferenceSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Help_Documentation,Commands.CommandList_Help);
	ReferenceSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Help_Overwatch, Commands.CommandList_Help);
	ReferenceSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Help_Roadmap, Commands.CommandList_Help);

	FToolMenuSection& SupportSection = Menu->FindOrAddSection("Support");
	SupportSection.Label = LOCTEXT("NHelp_Support", "Support");
	SupportSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Help_Discord, Commands.CommandList_Help);
	SupportSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Help_BugReport, Commands.CommandList_Help);
	SupportSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Help_Issues, Commands.CommandList_Help);
}

#undef LOCTEXT_NAMESPACE
