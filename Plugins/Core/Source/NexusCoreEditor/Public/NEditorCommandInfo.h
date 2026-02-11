// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

struct FNEditorCommandInfo
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

#define N_IMPLEMENT_GENERATE_COMMAND_INFO_MENU(Sections, Commands) \
	for (const auto& CommandSection : Sections) \
	{ \
		FToolMenuSection& Section = Menu->AddSection(CommandSection.Key, CommandSection.Value); \
		for (const auto& Command : Commands) \
		{ \
			if (Command.Value.Section != CommandSection.Key) continue; \
			if (Command.Value.bIsMenuEntry) \
			{ \
				Section.AddSubMenu(Command.Value.Identifier, Command.Value.DisplayName, \
					Command.Value.Tooltip, Command.Value.MenuChoice, false, Command.Value.Icon); \
			} \
			else \
			{ \
				FUIAction ButtonAction = FUIAction(Command.Value.Execute,Command.Value.CanExecute, \
					Command.Value.IsChecked, FIsActionButtonVisible()); \
				if (Command.Value.IsChecked.IsBound()) \
				{ \
					Section.AddMenuEntry(Command.Value.Identifier,  Command.Value.DisplayName, \
						Command.Value.Tooltip, Command.Value.Icon, \
						FToolUIActionChoice(ButtonAction), EUserInterfaceActionType::Check); \
				} \
				else \
				{ \
					Section.AddMenuEntry(Command.Value.Identifier,  Command.Value.DisplayName, \
						Command.Value.Tooltip, Command.Value.Icon, \
						FToolUIActionChoice(ButtonAction), EUserInterfaceActionType::Button); \
				} \
			} \
		} \
	}