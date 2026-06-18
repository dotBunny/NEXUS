// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Menus/NFixersMenu.h"

TMap<FName, FNMenuEntry> FNFixersMenu::Entries;

void FNFixersMenu::GenerateMenu(UToolMenu* Menu, bool bIsContextMenu)
{
	N_GENERATE_TOOLS_MENU(Entries)
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
}

void FNFixersMenu::RemoveMenuEntry(const FName Identifier)
{
	Entries.Remove(Identifier);
}