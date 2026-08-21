// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWorldAssemblyEditorStyle.h"
#include "Framework/Commands/UIAction.h"

class FUICommandList;

/**
 * The binding context every World Assembly editor command is declared against.
 *
 * Holds no commands of its own beyond the edit-mode entry point. RegisterCommands hands this context to each
 * category — FNCellEdModeRail and its peers — which declare their own commands, build their own
 * FUICommandList, and map their own actions. One context rather than one per category, so every command lands in a
 * single Editor Preferences > Keyboard Shortcuts section and saved chords stay keyed to the same name.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class FNWorldAssemblyEditorCommands final : public TCommands<FNWorldAssemblyEditorCommands>
{
public:
	/**
	 * One command paired with the delegates it binds to.
	 *
	 * @note IsChecked is what makes this usable by every category rather than only the ones without toggles. Leave it
	 *       unset for a plain button — an unbound delegate is exactly what FUIAction defaults to, so it costs a button
	 *       nothing to go through the same row a toggle does.
	 */
	struct FNCommandAction
	{
		TSharedPtr<FUICommandInfo> CommandInfo;
		FExecuteAction Execute;
		FCanExecuteAction CanExecute;
		FIsActionChecked IsChecked;
	};

	/**
	 * Build a category's command list from its actions.
	 *
	 * @param Actions Every command the category owns, paired with the delegates it binds to.
	 * @return A fresh list with all of them mapped, for the toolkit to fold into its own.
	 * @note Here rather than on FNEdModeRail because not every category has a rail —
	 *       FNWorldAssemblyEditorQuickAssembly is driven from the tool menu and the subsystem, and registers
	 *       through this same context.
	 */
	static TSharedRef<FUICommandList> MakeCommandList(const TArray<FNCommandAction>& Actions);

	FNWorldAssemblyEditorCommands()
		: TCommands<FNWorldAssemblyEditorCommands>(
			TEXT("NWorldAssemblyEditorCommands"),
			NSLOCTEXT("NexusWorldAssemblyEditor", "NWorldAssemblyEditorCommands", "NWorldAssemblyEditorCommands"),
			NAME_None,
			FNWorldAssemblyEditorStyle::GetStyleSetName())
	{
	}

	//~TCommands
	virtual void RegisterCommands() override;
	//End TCommands

	/** Toggle the World Assembly editor mode on the active level editor. */
	static void WorldAssemblyEdMode();
	/**
	 * @return true if the World Assembly-edit-mode entry should be shown in the current context.
	 * @note Stays here rather than moving to a category: entering the mode is not an action on a cell, junction,
	 *       world or organ — it is what puts the categories on screen in the first place.
	 */
	static bool WorldAssemblyEdMode_CanShow();
};
