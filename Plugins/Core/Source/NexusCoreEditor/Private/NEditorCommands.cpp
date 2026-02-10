// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorCommands.h"

#include "BlueprintEditor.h"
#include "NEditorSettings.h"
#include "NEditorUtils.h"
#include "NMetaUtils.h"
#include "DelayedEditorTasks/NLeakTestDelayedEditorTask.h"

TMap<FName, FNEditorCommandInfo> FNEditorCommands::WindowCommandInfo;
TMap<FName, FText> FNEditorCommands::WindowSections;

TMap<FName, FNEditorCommandInfo> FNEditorCommands::ToolsCommandInfo;
TMap<FName, FText> FNEditorCommands::ToolsSections;

void FNEditorCommands::RegisterCommands()
{
	// ReSharper disable StringLiteralTypo
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Help_Overwatch,"NCore.Help.OpenOverwatch",
	NSLOCTEXT("NexusCoreEditor", "Command_Help_OpenOverwatch", "Overwatch"),
	NSLOCTEXT("NexusCoreEditor","Command_Help_Overwatch_Desc", "Opens the GitHub project's development board in your browser."),
	FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.VisitCommunityHome"),
	EUserInterfaceActionType::Button, FInputChord());
	// ReSharper restore StringLiteralTypo
	
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Help_Issues,
	"NCore.Help.OpenIssues",
	NSLOCTEXT("NexusCoreEditor","Command_Help_OpenBugReport", "Issues"),
	NSLOCTEXT("NexusCoreEditor","Command_Help_OpenBugReport_Desc", "Opens the GitHub project's issue list in your browser."),
	FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.OpenIssueTracker"),
	EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Help_Discord,
	"NCore.Help.OpenDiscord",
	NSLOCTEXT("NexusCoreEditor","Command_Help_OpenDiscord", "Discord"),
	NSLOCTEXT("NexusCoreEditor","Command_Help_OpenDiscord_Desc", "Opens the Discord (dotBunny Support) invite link in your browser."),
	FSlateIcon(FNEditorStyle::GetStyleSetName(), "Command.OpenDiscordInviteLink"),
	EUserInterfaceActionType::Button, FInputChord());
	
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Help_BugReport,
	"NCore.Help.OpenBugReport",
	NSLOCTEXT("NexusCoreEditor","Command_Help_OpenBugReport", "Report a Bug"),
	NSLOCTEXT("NexusCoreEditor","Command_Help_OpenBugReport_Desc", "Opens the GitHub repository's bug report form in your browser."),
	FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.ReportABug"),
	EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Help_Roadmap,
		"NCore.Help.OpenRoadmap",
		NSLOCTEXT("NexusCoreEditor","Command_Help_OpenBugReport", "Roadmap"),
		NSLOCTEXT("NexusCoreEditor","Command_Help_OpenBugReport_Desc", "Opens the GitHub project's Roadmap in your browser."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.VisitOnlineLearning"),
		EUserInterfaceActionType::Button, FInputChord());	

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Help_Documentation,
		"NCore.Help.OpenDocumentation",
		NSLOCTEXT("NexusCoreEditor","Command_Help_OpenDocumentation", "Documentation"),
		NSLOCTEXT("NexusCoreEditor","Command_Help_OpenDocumentation_Desc", "Open the documentation in your browser."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Documentation"),
		EUserInterfaceActionType::Button, FInputChord());

	// Tools: Leak Check
	auto LeakCheckCommandInfo = FNEditorCommandInfo();
	LeakCheckCommandInfo.Identifier = "NCore.Tools.LeakCheck";
	LeakCheckCommandInfo.DisplayName = NSLOCTEXT("NexusCoreEditor","Command_Tools_LeakCheck", "Leak Check");
	LeakCheckCommandInfo.Tooltip = NSLOCTEXT("NexusCoreEditor","Command_Tools_LeakCheck_Desc", "Capture and process all UObjects over a period of 5 seconds to check for leaks."),
	LeakCheckCommandInfo.Icon = FSlateIcon(FNEditorStyle::GetStyleSetName(), "Command.LeakCheck");
	LeakCheckCommandInfo.Execute = FExecuteAction::CreateStatic(&UNLeakTestDelayedEditorTask::Create);
	AddToolCommand(LeakCheckCommandInfo);
	
	// Tools: Clean Logs
	auto CleanLogsCommandInfo = FNEditorCommandInfo();
	LeakCheckCommandInfo.Identifier = "NCore.Logs.CleanLogsFolder";
	LeakCheckCommandInfo.DisplayName = NSLOCTEXT("NexusCoreEditor","Command_Logs_CleanLogsFolder", "Clean Logs Folder");
	LeakCheckCommandInfo.Tooltip = NSLOCTEXT("NexusCoreEditor","Command_Logs_CleanLogsFolder_Desc", "Removes old logs from the Saved\\Logs folder."),
	LeakCheckCommandInfo.Icon = FSlateIcon(FNEditorStyle::GetStyleSetName(), "Command.CleanLogsFolder");
	LeakCheckCommandInfo.Execute = FExecuteAction::CreateStatic(&FNEditorCommands::OnWindowCleanLogsFolder);
	AddToolCommand(LeakCheckCommandInfo);

	
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_Tools_Profile_NetworkProfiler,
	"NCore.Tools.Profile.NetworkProfiler",
	NSLOCTEXT("NexusCoreEditor","Command_Tools_Profile_NetworkProfiler", "Network Profiler"),
	NSLOCTEXT("NexusCoreEditor","Command_Tools_Profile_NetworkProfiler", "Launch external NetworkProfiler tool."),
	FSlateIcon(FNEditorStyle::GetStyleSetName(), "Command.Visualizer"),
	EUserInterfaceActionType::Button, FInputChord());
	
	
	CommandList_Help = MakeShared<FUICommandList>();
	
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
			NSLOCTEXT("NexusCoreEditor","Command_Node_OpenExternalDocumentation", "External Documentation"),
			NSLOCTEXT("NexusCoreEditor","Command_Help_OpenRepository_Desc", "Open the external documentation (DocsURL) about this function."),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Documentation"),
			EUserInterfaceActionType::Button, FInputChord());

	CommandList_Node = MakeShared<FUICommandList>();

	CommandList_Node->MapAction(Get().CommandInfo_Node_ExternalDocumentation,
		FExecuteAction::CreateStatic(&FNEditorCommands::OnNodeExternalDocumentation),
		FCanExecuteAction::CreateStatic(&FNEditorCommands::NodeExternalDocumentation_CanExecute));
	
	CommandList_Tools = MakeShared<FUICommandList>();
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

void FNEditorCommands::OnWindowCleanLogsFolder()
{
	TArray<FString> FilePaths;
	IFileManager& FileManager = IFileManager::Get();
	
	TArray<FString> Searches;
	Searches.Add(TEXT("*-backup-*")); // Backups
	Searches.Add(TEXT("NEXUS_Compare*")); // NEXUS Compares
	Searches.Add(TEXT("NEXUS_Snapshot*")); // NEXUS Snapshots
	Searches.Add(TEXT("*VersionSelect*")); // UE Version Selector
	Searches.Add(FString::Printf(TEXT("%s_*"), FApp::GetProjectName())); // Project secondary logs

	const FString ProjectLogDir = FPaths::ProjectLogDir();
	int DeleteCount = 0;

	for (const FString& Search : Searches)
	{
		FileManager.FindFilesRecursive(FilePaths, *ProjectLogDir, *Search, true, false);
		for (const FString& File : FilePaths)
		{
			FileManager.Delete(*File, false, true);
			DeleteCount++;
		}
	}
	
	if (DeleteCount > 0)
	{
		UE_LOG(LogNexusCoreEditor, Log, TEXT("Deleted %i files from %s."), DeleteCount, *FPaths::ProjectLogDir());
	}
	else
	{
		UE_LOG(LogNexusCoreEditor, Warning, TEXT("No files found to delete from %s."), *FPaths::ProjectLogDir());
	}
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
				NSLOCTEXT("NexusCoreEditor", "ProjectLevels", "Project Levels"),
				NSLOCTEXT("NexusCoreEditor", "ProjectLevels_Tooltip", "A pre-defined list of levels related to the project."),
				FNewToolMenuDelegate::CreateStatic(&FillProjectLevelsSubMenu),
				false,
				FSlateIcon(FNEditorStyle::GetStyleSetName(), "Command.ProjectLevels")
			);
	}
	
	// Dynamic NEXUS Windows
	UToolMenu* WindowsMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
	FToolMenuSection& LevelEditorSection = WindowsMenu->FindOrAddSection("LevelEditor");
	LevelEditorSection.AddSubMenu(
			"NEXUS_Windows",
			NSLOCTEXT("NexusCoreEditor", "NWindows", "NEXUS"),
			NSLOCTEXT("NexusCoreEditor", "NWindows_ToolTip", "EUW/Windows added by parts of NEXUS."),
			FNewToolMenuDelegate::CreateStatic(&FillWindowMenu, true),
			false,
			FSlateIcon(FNEditorStyle::GetStyleSetName(), "NEXUS.Icon")
		);
	
	// Dynamic NEXUS Tools
	if (UToolMenu* ToolMenus = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools"))
	{
		FToolMenuSection& ToolsSection = ToolMenus->FindOrAddSection("Tools");
		ToolsSection.AddSubMenu(
			"NEXUS_Tools",
			NSLOCTEXT("NexusCoreEditor", "NTools", "NEXUS"),
			NSLOCTEXT("NexusCoreEditor", "NTools_ToolTip", "Tools added by NEXUS."),
			FNewToolMenuDelegate::CreateStatic(&FillToolMenu, true),
			false,
			FSlateIcon(FNEditorStyle::GetStyleSetName(), "NEXUS.Icon")
		);
	}
	
	// Add in NetworkProfiler menu option if its present
	if (HasToolsProfileNetworkProfiler())
	{
		if (UToolMenu* ProfileMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools.Profile"))
		{
			FToolMenuSection& ExternalMenu = ProfileMenu->FindOrAddSection("External");
			ExternalMenu.Label = NSLOCTEXT("NexusCoreEditor", "NLevelEditorToolsExternal", "External");

			ExternalMenu.AddMenuEntryWithCommandList(Commands.CommandInfo_Tools_Profile_NetworkProfiler, Commands.CommandList_Tools);
		}
	}
	
	// Help Menu Submenu
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Help"))
	{
		FToolMenuSection& ApplicationSection = Menu->FindOrAddSection(FName("Reference"));
		ApplicationSection.AddSubMenu(
				"NEXUS",
				NSLOCTEXT("NexusCoreEditor", "NHelp", "NEXUS"),
				NSLOCTEXT("NexusCoreEditor", "NHelp_Tooltip", "Help related to NEXUS"),
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
	FToolMenuSection& ProjectLevelsSection = Menu->AddSection("ProjectLevels", NSLOCTEXT("NexusCoreEditor", "ProjectLevels", ""));
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

void FNEditorCommands::FillEditorCommandMenu(UToolMenu* Menu, bool bIsContextMenu, TMap<FName, FText>& Sections, TMap<FName, FNEditorCommandInfo> Actions)
{
	for (auto CommandSection : Sections)
	{
		FToolMenuSection& Section = Menu->AddSection(CommandSection.Key, CommandSection.Value);
		for (auto CommandAction : Actions)
		{
			if (CommandAction.Value.Section != CommandSection.Key) continue;
			
			FUIAction ButtonAction = FUIAction(CommandAction.Value.Execute,CommandAction.Value.CanExecute, 
				CommandAction.Value.IsChecked, FIsActionButtonVisible());
		
			if (CommandAction.Value.IsChecked.IsBound())
			{
				Section.AddMenuEntry(CommandAction.Value.Identifier,  CommandAction.Value.DisplayName, 
				CommandAction.Value.Tooltip, CommandAction.Value.Icon,
				FToolUIActionChoice(ButtonAction), EUserInterfaceActionType::Check);
			}
			else
			{
				Section.AddMenuEntry(CommandAction.Value.Identifier,  CommandAction.Value.DisplayName, 
				CommandAction.Value.Tooltip, CommandAction.Value.Icon,
				FToolUIActionChoice(ButtonAction), EUserInterfaceActionType::Button);
			}
		}
	}
}

void FNEditorCommands::AddWindowCommand(FNEditorCommandInfo CommandInfo)
{
	if (!WindowCommandInfo.Contains(CommandInfo.Identifier))
	{
		WindowCommandInfo.Add(CommandInfo.Identifier, CommandInfo);
		WindowCommandInfo.KeySort([](const FName A, const FName B)
			{
				return A.Compare(B) < 0;
			});
	}
	else
	{
		WindowCommandInfo[CommandInfo.Identifier] = CommandInfo;
	}
	
	if (!WindowSections.Contains(CommandInfo.Section))
	{
		WindowSections.Add(CommandInfo.Section, FText::FromName(CommandInfo.Section));
		WindowSections.KeySort([](const FName A, const FName B)
			{
				return A.Compare(B) < 0;
			});
	}
}

void FNEditorCommands::RemoveWindowCommand(const FName Identifier)
{
	if (WindowCommandInfo.Contains(Identifier))
	{
		WindowCommandInfo.Remove(Identifier);
	}
}

void FNEditorCommands::AddToolCommand(FNEditorCommandInfo CommandInfo)
{
	if (!ToolsCommandInfo.Contains(CommandInfo.Identifier))
	{
		ToolsCommandInfo.Add(CommandInfo.Identifier, CommandInfo);
		ToolsCommandInfo.KeySort([](const FName A, const FName B)
			{
				return A.Compare(B) < 0;
			});
	}
	else
	{
		ToolsCommandInfo[CommandInfo.Identifier] = CommandInfo;
	}
	
	if (!ToolsSections.Contains(CommandInfo.Section))
	{
		ToolsSections.Add(CommandInfo.Section, FText::FromName(CommandInfo.Section));
		ToolsSections.KeySort([](const FName A, const FName B)
			{
				return A.Compare(B) < 0;
			});
	}
	
	
}

void FNEditorCommands::RemoveToolCommand(FName Identifier)
{
	if (ToolsCommandInfo.Contains(Identifier))
	{
		ToolsCommandInfo.Remove(Identifier);
	}
}

void FNEditorCommands::FillHelpSubMenu(UToolMenu* Menu)
{
	const FNEditorCommands Commands = Get();

	FToolMenuSection& ReferenceSection = Menu->FindOrAddSection("Reference");
	ReferenceSection.Label = NSLOCTEXT("NexusCoreEditor", "NHelp_Reference", "Reference");
	ReferenceSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Help_Documentation,Commands.CommandList_Help);
	ReferenceSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Help_Overwatch, Commands.CommandList_Help);
	ReferenceSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Help_Roadmap, Commands.CommandList_Help);

	FToolMenuSection& SupportSection = Menu->FindOrAddSection("Support");
	SupportSection.Label = NSLOCTEXT("NexusCoreEditor", "NHelp_Support", "Support");
	SupportSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Help_Discord, Commands.CommandList_Help);
	SupportSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Help_BugReport, Commands.CommandList_Help);
	SupportSection.AddMenuEntryWithCommandList(Commands.CommandInfo_Help_Issues, Commands.CommandList_Help);
}
