// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NFixersMenu.h"
#include "NToolsEditorStyle.h"

TMap<FName, FNToolsMenuItem> FNFixersMenu::Items;
TMap<FName, FText> FNFixersMenu::Sections;

void FNFixersMenu::Register()
{
	// Add to folder to ContentBrowser
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.FolderContextMenu"))
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("PathContextBulkOperations");
		Section.AddSubMenu(
				"NFixersBulkOperations",
				NSLOCTEXT("NexusToolsEditor", "ContextMenu_FindAndFix", "Find & Fix"),
				NSLOCTEXT("NexusToolsEditor", "ContextMenu_FindAndFix_ToolTip", "Find and fix operations on selected content."),
				FNewToolMenuDelegate::CreateStatic(&GenerateMenu, true),
				false,
				FSlateIcon(FNToolsEditorStyle::GetStyleSetName(), "Command.FindAndFix")
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
	N_IMPLEMENT_GENERATE_TOOLS_MENU(Sections, Items)
}

void FNFixersMenu::AddCommand(const FNToolsMenuItem& Item)
{
	if (!Items.Contains(Item.Identifier))
	{
		Items.Add(Item.Identifier, Item);
		Items.KeySort([](const FName A, const FName B)
			{
				return A.Compare(B) < 0;
			});
	}
	else
	{
		Items[Item.Identifier] = Item;
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

void FNFixersMenu::RemoveCommand(const FName Identifier)
{
	if (Items.Contains(Identifier))
	{
		Items.Remove(Identifier);
	}
}