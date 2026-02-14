// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

struct FNMenuEntry
{
	FName Section = TEXT("General");
	FName Identifier;
	FText DisplayName;
	FText Tooltip;
	FExecuteAction Execute;
	FCanExecuteAction CanExecute;
	FIsActionChecked IsChecked;
	bool bIsMenuEntry = false;
	FNewToolMenuChoice MenuChoice;
	FSlateIcon Icon;
};

#define N_IMPLEMENT_GENERATE_TOOLS_MENU(Sections, MenuEntries) \
	for (const auto& Section : Sections) \
	{ \
		FToolMenuSection& MenuSection = Menu->AddSection(Section.Key, Section.Value); \
		for (const auto& Entry : MenuEntries) \
		{ \
			if (Entry.Value.Section != Section.Key) continue; \
			if (Entry.Value.bIsMenuEntry) \
			{ \
				MenuSection.AddSubMenu(Entry.Value.Identifier, Entry.Value.DisplayName, \
					Entry.Value.Tooltip, Entry.Value.MenuChoice, false, Entry.Value.Icon); \
			} \
			else \
			{ \
				FUIAction ButtonAction = FUIAction(Entry.Value.Execute,Entry.Value.CanExecute, \
					Entry.Value.IsChecked, FIsActionButtonVisible()); \
				if (Entry.Value.IsChecked.IsBound()) \
				{ \
					MenuSection.AddMenuEntry(Entry.Value.Identifier,  Entry.Value.DisplayName, \
						Entry.Value.Tooltip, Entry.Value.Icon, \
						FToolUIActionChoice(ButtonAction), EUserInterfaceActionType::Check); \
				} \
				else \
				{ \
					MenuSection.AddMenuEntry(Entry.Value.Identifier,  Entry.Value.DisplayName, \
						Entry.Value.Tooltip, Entry.Value.Icon, \
						FToolUIActionChoice(ButtonAction), EUserInterfaceActionType::Button); \
				} \
			} \
		} \
	}