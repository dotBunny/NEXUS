// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NFixersEditorBulkOperations.h"

#include "NEditorCommands.h"
#include "NFixersEditorStyle.h"

TMap<FName, FNEditorCommandInfo> FNFixersEditorBulkOperations::CommandInfo;
TMap<FName, FText> FNFixersEditorBulkOperations::Sections;

void FNFixersEditorBulkOperations::Register()
{
	// Add to folder to ContentBrowser
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.FolderContextMenu"))
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("PathContextBulkOperations");
		Section.AddSubMenu(
				"NFixersBulkOperations",
				NSLOCTEXT("NexusFixersEditor", "ContextMenu_FindAndFix", "Find & Fix"),
				NSLOCTEXT("NexusFixersEditor", "ContextMenu_FindAndFix_ToolTip", "Find and fix operations on selected content."),
				FNewToolMenuDelegate::CreateStatic(&FillContextBulkOperationsMenu, true),
				false,
				FSlateIcon(FNFixersEditorStyle::GetStyleSetName(), "Command.FindAndFix")
			);
	}
}

void FNFixersEditorBulkOperations::Unregister()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (ToolMenus != nullptr)
	{
		ToolMenus->RemoveEntry("ContentBrowser.FolderContextMenu", "PathContextBulkOperations", "NFixersBulkOperations");
	}
}

void FNFixersEditorBulkOperations::FillContextBulkOperationsMenu(UToolMenu* Menu, bool bIsContextMenu)
{
	FNEditorCommands::FillMenu(Menu, bIsContextMenu, Sections, CommandInfo);
}


void FNFixersEditorBulkOperations::AddCommand(const FNEditorCommandInfo& Command)
{
	if (!CommandInfo.Contains(Command.Identifier))
	{
		CommandInfo.Add(Command.Identifier, Command);
		CommandInfo.KeySort([](const FName A, const FName B)
			{
				return A.Compare(B) < 0;
			});
	}
	else
	{
		CommandInfo[Command.Identifier] = Command;
	}
	
	if (!Sections.Contains(Command.Section))
	{
		Sections.Add(Command.Section, FText::FromName(Command.Section));
		Sections.KeySort([](const FName A, const FName B)
			{
				return A.Compare(B) < 0;
			});
	}
}

void FNFixersEditorBulkOperations::RemoveCommand(const FName Identifier)
{
	if (CommandInfo.Contains(Identifier))
	{
		CommandInfo.Remove(Identifier);
	}
}