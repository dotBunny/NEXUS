#include "Menus/NToolsMenu.h"


TMap<FName, FNMenuEntry> FNToolsMenu::Entries;
TMap<FName, FText> FNToolsMenu::Sections;

void FNToolsMenu::AddMenuEntry(const FNMenuEntry& Item)
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

void FNToolsMenu::RemoveMenuEntry(const FName Identifier)
{
	if (Entries.Contains(Identifier))
	{
		Entries.Remove(Identifier);
	}
}

void FNToolsMenu::GenerateMenu(UToolMenu* Menu, bool bIsContextMenu)
{
	N_IMPLEMENT_GENERATE_TOOLS_MENU(Sections, Entries)
}

