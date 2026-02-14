// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorCommands.h"
#include "BlueprintEditor.h"
#include "NEditorUtils.h"
#include "NMetaUtils.h"

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

void FNEditorCommands::AddMenuEntries()
{
	// Help Menu Submenu
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Help"))
	{
		FToolMenuSection& ApplicationSection = Menu->FindOrAddSection(FName("Reference"));
		ApplicationSection.AddSubMenu(
				"NEXUS",
				NSLOCTEXT("NexusCoreEditor", "NHelp", "NEXUS"),
				NSLOCTEXT("NexusCoreEditor", "NHelp_Tooltip", "Help related to NEXUS"),
				FNewToolMenuDelegate::CreateStatic(&GenerateHelpSubMenu),
				false,
				FSlateIcon(FNEditorStyle::GetStyleSetName(), "NEXUS.Icon")
			);
	}
}

void FNEditorCommands::RemoveMenuEntries()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (ToolMenus)
	{
		ToolMenus->RemoveEntry("LevelEditor.MainMenu.Help", "Reference", "NEXUS");
	}
}

void FNEditorCommands::GenerateHelpSubMenu(UToolMenu* Menu)
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
