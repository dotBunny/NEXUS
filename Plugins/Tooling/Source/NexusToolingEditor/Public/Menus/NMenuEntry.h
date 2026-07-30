// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Generic description of a single tool-menu entry registered with FNToolsMenu / FNFixersMenu.
 * bIsMenuEntry distinguishes sub-menu entries (use MenuChoice) from action entries (use
 * Execute/CanExecute/IsChecked). A bound IsChecked flips the entry to a toggleable check-item.
 */
struct FNMenuEntry
{
	/** Section identifier used to group related entries within the menu. */
	FName Section = TEXT("General");

	/** Stable identifier used for lookup and removal. */
	FName Identifier;

	/** Human-readable label shown in the menu. */
	FText DisplayName;

	/** Tooltip shown when hovering the entry. */
	FText Tooltip;

	/** Action invoked when the entry is clicked (ignored when bIsMenuEntry is true). */
	FExecuteAction Execute;

	/** Optional predicate that gates Execute; unbound delegates always allow execution. */
	FCanExecuteAction CanExecute;

	/** Optional predicate that promotes the entry to a toggleable check-item. */
	FIsActionChecked IsChecked;

	/** When true the entry is a sub-menu opened via MenuChoice instead of an action button. */
	bool bIsMenuEntry = false;

	/** Factory used to populate the sub-menu's contents when bIsMenuEntry is true. */
	FNewToolMenuChoice MenuChoice;

	/** Optional icon shown next to DisplayName. */
	FSlateIcon Icon;
};

// Sections are derived from the entries themselves at generation time (a section exists only while at
// least one entry references it), so there is no separate section list that can drift out of sync as
// entries are added and removed.
#define N_GENERATE_TOOLS_MENU(MenuEntries) \
	{ \
		TArray<FName> SectionOrder; \
		for (const auto& Entry : MenuEntries) \
		{ \
			SectionOrder.AddUnique(Entry.Value.Section); \
		} \
		SectionOrder.Sort([](const FName A, const FName B) \
			{ \
				return A.Compare(B) < 0; \
			}); \
		for (const FName& SectionName : SectionOrder) \
		{ \
			FToolMenuSection& MenuSection = Menu->AddSection(SectionName, FText::FromName(SectionName)); \
			for (const auto& Entry : MenuEntries) \
			{ \
				if (Entry.Value.Section != SectionName) continue; \
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
		} \
	}