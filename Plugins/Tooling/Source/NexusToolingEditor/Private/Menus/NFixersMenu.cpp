// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Menus/NFixersMenu.h"
#include "NToolingEditorStyle.h"

TMap<FName, FNMenuEntry> FNFixersMenu::Entries;
TMap<FName, FText> FNFixersMenu::Sections;

void FNFixersMenu::Register()
{
	// Add to folder to ContentBrowser
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.FolderContextMenu"))
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("PathContextBulkOperations");
		Section.AddSubMenu(
				"NFixersBulkOperations",
				NSLOCTEXT("NexusToolingEditor", "ContextMenu_FindAndFix", "Find & Fix"),
				NSLOCTEXT("NexusToolingEditor", "ContextMenu_FindAndFix_ToolTip", "Find and fix operations on selected content."),
				FNewToolMenuDelegate::CreateStatic(&GenerateMenu, true),
				false,
				FSlateIcon(FNToolingEditorStyle::GetStyleSetName(), "Command.FindAndFix")
			);
	}
}

void FNFixersMenu::Unregister()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (ToolMenus != nullptr)
	{
		ToolMenus->RemoveEntry("ContentBrowser.FolderContextMenu", "PathContextBulkOperations", "NFixersBulkOperations");
	}
}

void FNFixersMenu::GenerateMenu(UToolMenu* Menu, bool bIsContextMenu)
{
	N_IMPLEMENT_GENERATE_TOOLS_MENU(Sections, Entries)
}

void FNFixersMenu::AddMenuEntry(const FNMenuEntry& Item)
{
	if (!Entries.Contains(Item.Identifier))
	{
		Entries.Add(Item.Identifier, Item);
		Entries.KeySort([](const FName A, const FName B)
			{
				return A.Compare(B) < 0;
			});
	}
	else
	{
		Entries[Item.Identifier] = Item;
	}
	
	if (!Sections.Contains(Item.Section))
	{
		Sections.Add(Item.Section, FText::FromName(Item.Section));
		Sections.KeySort([](const FName A, const FName B)
			{
				return A.Compare(B) < 0;
			});
	}
}

void FNFixersMenu::RemoveMenuEntry(const FName Identifier)
{
	if (Entries.Contains(Identifier))
	{
		Entries.Remove(Identifier);
	}
}

void FNFixersMenu::AddKnownEntries()
{
}
