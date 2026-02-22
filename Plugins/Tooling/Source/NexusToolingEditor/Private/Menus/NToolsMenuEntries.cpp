// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Menus/NToolsMenuEntries.h"
#include "NToolingEditorStyle.h"
#include "DelayedEditorTasks/NLeakTestDelayedEditorTask.h"
#include "Menus/NMenuEntry.h"
#include "Menus/NToolsMenu.h"

void FNToolsMenuEntries::AddMenuEntries()
{
	// Leak Check
	auto LeakCheckMenuEntry = FNMenuEntry();
	LeakCheckMenuEntry.Identifier = "LeakCheck";
	LeakCheckMenuEntry.DisplayName = NSLOCTEXT("NexusToolingEditor","LeakCheck", "Leak Check");
	LeakCheckMenuEntry.Tooltip = NSLOCTEXT("NexusToolingEditor","LeakCheck_Tooltip", "Capture and process all UObjects over a defined period to check for leaks."),
	LeakCheckMenuEntry.Icon = FSlateIcon(FNToolingEditorStyle::GetStyleSetName(), "Command.LeakCheck");
	LeakCheckMenuEntry.Execute = FExecuteAction::CreateStatic(&UNLeakTestDelayedEditorTask::Create);
	FNToolsMenu::AddMenuEntry(LeakCheckMenuEntry);
	
	// Clean Logs
	auto CleanLogsMenuEntry = FNMenuEntry();
	CleanLogsMenuEntry.Identifier = "CleanLogsFolder";
	CleanLogsMenuEntry.DisplayName = NSLOCTEXT("NexusToolingEditor","CleanLogsFolder", "Clean Logs Folder");
	CleanLogsMenuEntry.Tooltip = NSLOCTEXT("NexusToolingEditor","CleanLogsFolder_Tooltip", "Removes old logs from the Saved\\Logs folder."),
	CleanLogsMenuEntry.Icon = FSlateIcon(FNToolingEditorStyle::GetStyleSetName(), "Command.CleanLogsFolder");
	CleanLogsMenuEntry.Execute = FExecuteAction::CreateStatic(&OnWindowCleanLogsFolder);
	FNToolsMenu::AddMenuEntry(CleanLogsMenuEntry);
}

void FNToolsMenuEntries::OnWindowCleanLogsFolder()
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
		UE_LOG(LogNexusToolingEditor, Log, TEXT("Deleted %i files from %s."), DeleteCount, *FPaths::ProjectLogDir());
	}
	else
	{
		UE_LOG(LogNexusToolingEditor, Warning, TEXT("No files found to delete from %s."), *FPaths::ProjectLogDir());
	}
}

