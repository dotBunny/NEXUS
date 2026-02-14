#include "NToolsEditorMenu.h"

#include "NEditorStyle.h"
#include "DelayedEditorTasks/NLeakTestDelayedEditorTask.h"

TMap<FName, FNEditorCommandInfo> FNToolsEditorMenu::CommandInfos;
TMap<FName, FText> FNToolsEditorMenu::Sections;

void FNToolsEditorMenu::AddCommand(FNEditorCommandInfo CommandInfo)
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

void FNToolsEditorMenu::RemoveCommand(const FName Identifier)
{
	if (CommandInfos.Contains(Identifier))
	{
		CommandInfos.Remove(Identifier);
	}
}


void FNToolsEditorMenu::GenerateMenu(UToolMenu* Menu, bool bIsContextMenu)
{
	N_IMPLEMENT_GENERATE_COMMAND_INFO_MENU(Sections, CommandInfos)
}

