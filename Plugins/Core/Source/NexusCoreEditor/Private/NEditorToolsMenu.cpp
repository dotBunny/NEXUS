#include "NEditorToolsMenu.h"

#include "NEditorStyle.h"
#include "DelayedEditorTasks/NLeakTestDelayedEditorTask.h"

TMap<FName, FNEditorCommandInfo> FNEditorToolsMenu::CommandInfos;
TMap<FName, FText> FNEditorToolsMenu::Sections;

void FNEditorToolsMenu::AddCommand(FNEditorCommandInfo CommandInfo)
{
	if (!CommandInfos.Contains(CommandInfo.Identifier))
	{
		CommandInfos.Add(CommandInfo.Identifier, CommandInfo);
		CommandInfos.KeySort([](const FName A, const FName B)
			{
				return A.Compare(B) < 0;
			});
	}
	else
	{
		CommandInfos[CommandInfo.Identifier] = CommandInfo;
	}
	
	if (!Sections.Contains(CommandInfo.Section))
	{
		Sections.Add(CommandInfo.Section, FText::FromName(CommandInfo.Section));
		Sections.KeySort([](const FName A, const FName B)
			{
				return A.Compare(B) < 0;
			});
	}
}

void FNEditorToolsMenu::RemoveCommand(const FName Identifier)
{
	if (CommandInfos.Contains(Identifier))
	{
		CommandInfos.Remove(Identifier);
	}
}

void FNEditorToolsMenu::RegisterCommands()
{
	// Leak Check
	auto LeakCheckCommandInfo = FNEditorCommandInfo();
	LeakCheckCommandInfo.Identifier = "NCore.Tools.LeakCheck";
	LeakCheckCommandInfo.DisplayName = NSLOCTEXT("NexusCoreEditor","Command_Tools_LeakCheck", "Leak Check");
	LeakCheckCommandInfo.Tooltip = NSLOCTEXT("NexusCoreEditor","Command_Tools_LeakCheck_Desc", "Capture and process all UObjects over a period of 5 seconds to check for leaks."),
	LeakCheckCommandInfo.Icon = FSlateIcon(FNEditorStyle::GetStyleSetName(), "Command.LeakCheck");
	LeakCheckCommandInfo.Execute = FExecuteAction::CreateStatic(&UNLeakTestDelayedEditorTask::Create);
	AddCommand(LeakCheckCommandInfo);
	
	// Clean Logs
	auto CleanLogsCommandInfo = FNEditorCommandInfo();
	LeakCheckCommandInfo.Identifier = "NCore.Logs.CleanLogsFolder";
	LeakCheckCommandInfo.DisplayName = NSLOCTEXT("NexusCoreEditor","Command_Logs_CleanLogsFolder", "Clean Logs Folder");
	LeakCheckCommandInfo.Tooltip = NSLOCTEXT("NexusCoreEditor","Command_Logs_CleanLogsFolder_Desc", "Removes old logs from the Saved\\Logs folder."),
	LeakCheckCommandInfo.Icon = FSlateIcon(FNEditorStyle::GetStyleSetName(), "Command.CleanLogsFolder");
	LeakCheckCommandInfo.Execute = FExecuteAction::CreateStatic(&OnWindowCleanLogsFolder);
	AddCommand(LeakCheckCommandInfo);
}

void FNEditorToolsMenu::GenerateMenu(UToolMenu* Menu, bool bIsContextMenu)
{
	N_IMPLEMENT_GENERATE_COMMAND_INFO_MENU(Sections, CommandInfos)
}

void FNEditorToolsMenu::OnWindowCleanLogsFolder()
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
