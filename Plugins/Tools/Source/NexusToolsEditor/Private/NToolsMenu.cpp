#include "NToolsMenu.h"


TMap<FName, FNToolsMenuItem> FNToolsMenu::Items;
TMap<FName, FText> FNToolsMenu::Sections;

void FNToolsMenu::AddCommand(const FNToolsMenuItem& Item)
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

void FNToolsMenu::RemoveCommand(const FName Identifier)
{
	if (Items.Contains(Identifier))
	{
		Items.Remove(Identifier);
	}
}


void FNToolsMenu::GenerateMenu(UToolMenu* Menu, bool bIsContextMenu)
{
	N_IMPLEMENT_GENERATE_TOOLS_MENU(Sections, Items)
}

