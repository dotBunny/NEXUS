// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NToolsEditorCommands.h"

#include "NEditorStyle.h"
#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetSubsystem.h"
#include "NToolsEditorMenu.h"
#include "NToolsEditorMinimal.h"
#include "NToolsEditorStyle.h"
#include "DelayedEditorTasks/NLeakTestDelayedEditorTask.h"

void FNToolsEditorCommands::AddMenuEntries()
{
	// Dynamic NEXUS Tools
	if (UToolMenu* ToolMenus = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools"))
	{
		FToolMenuSection& ToolsSection = ToolMenus->FindOrAddSection("Tools");
		FToolMenuEntry Entry = FToolMenuEntry::InitSubMenu(
			"NEXUS",
			NSLOCTEXT("NexusCoreEditor", "NTools", "NEXUS"),
			NSLOCTEXT("NexusCoreEditor", "NTools_ToolTip", "Tools added by NEXUS that don't seem to fit anywhere else."),
			FNewToolMenuDelegate::CreateStatic(&FNToolsEditorMenu::GenerateMenu, false),
			false,
			FSlateIcon(FNEditorStyle::GetStyleSetName(), "NEXUS.Icon")
		);
		Entry.InsertPosition = FToolMenuInsert("FindInBlueprints", EToolMenuInsertType::After);
		ToolsSection.AddEntry(Entry);
	}
	
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools.Debug"))
	{
		// Add Collision Visualizer
		FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
			NEXUS::ToolsEditor::CollisionVisualizer::Identifier,
			NSLOCTEXT("NexusUIEditor", "Create_EUW_CollisionVisualizer_DisplayName", "Collision Visualizer"),
			NSLOCTEXT("NexusUIEditor", "Create_EUW_CollisionVisualizer_Tooltip", "Opens the NEXUS: Collision Visualizer window."),
			FSlateIcon(FNToolsEditorStyle::GetStyleSetName(), NEXUS::ToolsEditor::CollisionVisualizer::Icon),
			FExecuteAction::CreateStatic(&FNToolsEditorCommands::CreateCollisionVisualizerWindow));
		Entry.InsertPosition = FToolMenuInsert(NAME_None, EToolMenuInsertType::First);
		Menu->AddMenuEntry("Debug", Entry);
	}
	
	// Add in NetworkProfiler menu option if its present
	if (HasToolsProfileNetworkProfiler())
	{
		if (UToolMenu* ProfileMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools.Profile"))
		{
			FToolMenuSection& ExternalMenu = ProfileMenu->FindOrAddSection("External");
			ExternalMenu.Label = NSLOCTEXT("NexusToolsEditor", "NLevelEditorToolsExternal", "External");
			
			ExternalMenu.AddMenuEntry(
			NEXUS::ToolsEditor::NetworkProfiler::Identifier,
			NSLOCTEXT("NexusToolsEditor", "Command_Tools_Profile_NetworkProfiler", "Network Profiler"),
			NSLOCTEXT("NexusToolsEditor", "Command_Tools_Profile_NetworkProfiler", "Launch external NetworkProfiler tool."),
			FSlateIcon(FNToolsEditorStyle::GetStyleSetName(), "Command.Visualizer"),
			FExecuteAction::CreateStatic(&FNToolsEditorCommands::OnToolsProfileNetworkProfiler));
		}
	}

	
	// Leak Check
	auto LeakCheckCommandInfo = FNEditorCommandInfo();
	LeakCheckCommandInfo.Identifier = "NCore.Tools.LeakCheck";
	LeakCheckCommandInfo.DisplayName = NSLOCTEXT("NexusCoreEditor","Command_Tools_LeakCheck", "Leak Check");
	LeakCheckCommandInfo.Tooltip = NSLOCTEXT("NexusCoreEditor","Command_Tools_LeakCheck_Desc", "Capture and process all UObjects over a period of 5 seconds to check for leaks."),
	LeakCheckCommandInfo.Icon = FSlateIcon(FNToolsEditorStyle::GetStyleSetName(), "Command.LeakCheck");
	LeakCheckCommandInfo.Execute = FExecuteAction::CreateStatic(&UNLeakTestDelayedEditorTask::Create);
	FNToolsEditorMenu::AddCommand(LeakCheckCommandInfo);
	
	// Clean Logs
	auto CleanLogsCommandInfo = FNEditorCommandInfo();
	CleanLogsCommandInfo.Identifier = "NCore.Logs.CleanLogsFolder";
	CleanLogsCommandInfo.DisplayName = NSLOCTEXT("NexusCoreEditor","Command_Logs_CleanLogsFolder", "Clean Logs Folder");
	CleanLogsCommandInfo.Tooltip = NSLOCTEXT("NexusCoreEditor","Command_Logs_CleanLogsFolder_Desc", "Removes old logs from the Saved\\Logs folder."),
	CleanLogsCommandInfo.Icon = FSlateIcon(FNToolsEditorStyle::GetStyleSetName(), "Command.CleanLogsFolder");
	CleanLogsCommandInfo.Execute = FExecuteAction::CreateStatic(&OnWindowCleanLogsFolder);
	FNToolsEditorMenu::AddCommand(CleanLogsCommandInfo);
}

void FNToolsEditorCommands::RemoveMenuEntries()
{
	UToolMenus* Menu = UToolMenus::Get();
	if (Menu)
	{
		// Remove Collision Visualizer
		Menu->RemoveEntry("LevelEditor.MainMenu.Tools.Debug", "Debug", 
			NEXUS::ToolsEditor::CollisionVisualizer::Identifier);
		
		Menu->RemoveEntry("LevelEditor.MainMenu.Tools", "Tools", "NEXUS");
		
		Menu->RemoveSection("LevelEditor.MainMenu.Tools.Profile", "External");
	}
}


void FNToolsEditorCommands::CreateCollisionVisualizerWindow()
{
	UNEditorUtilityWidget::SpawnTab(
		NEXUS::ToolsEditor::CollisionVisualizer::Path, 
		NEXUS::ToolsEditor::CollisionVisualizer::Identifier);
}

bool FNToolsEditorCommands::HasCollisionVisualizerWindow()
{
	UNEditorUtilityWidgetSubsystem* System = UNEditorUtilityWidgetSubsystem::Get();
	if (System == nullptr) return false;
	return System->HasWidget(NEXUS::ToolsEditor::CollisionVisualizer::Identifier);
}

void FNToolsEditorCommands::OnToolsProfileNetworkProfiler()
{
	const FString ExecutablePath = FPaths::Combine(FNEditorUtils::GetEngineBinariesPath(), "DotNet", "NetworkProfiler.exe");
	constexpr bool bLaunchDetached = true;
	constexpr bool bLaunchHidden = false;
	constexpr bool bLaunchReallyHidden = false;
	const FProcHandle ProcHandle = FPlatformProcess::CreateProc(*ExecutablePath, TEXT(""), bLaunchDetached,
		bLaunchHidden, bLaunchReallyHidden, nullptr, 0, nullptr, nullptr, nullptr);
	if (!ProcHandle.IsValid())
	{
		UE_LOG(LogNexusToolsEditor, Error, TEXT("Unable to launch NetworkProfiler."))
	}
}

bool FNToolsEditorCommands::HasToolsProfileNetworkProfiler()
{
	return FPaths::FileExists(FPaths::Combine(FNEditorUtils::GetEngineBinariesPath(), "DotNet", "NetworkProfiler.exe"));
}

void FNToolsEditorCommands::OnWindowCleanLogsFolder()
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
		UE_LOG(LogNexusToolsEditor, Log, TEXT("Deleted %i files from %s."), DeleteCount, *FPaths::ProjectLogDir());
	}
	else
	{
		UE_LOG(LogNexusToolsEditor, Warning, TEXT("No files found to delete from %s."), *FPaths::ProjectLogDir());
	}
}
