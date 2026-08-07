// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWorldAssemblyEditorStyle.h"

/**
 * The binding context every World Assembly editor command is declared against.
 *
 * Holds no commands of its own beyond the edit-mode entry point. RegisterCommands hands this context to each
 * category — FNWorldAssemblyEditorCellCommands and its peers — which declare their own commands, build their own
 * FUICommandList, and map their own actions. One context rather than one per category, so every command lands in a
 * single Editor Preferences > Keyboard Shortcuts section and saved chords stay keyed to the same name.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class FNWorldAssemblyEditorCommands final : public TCommands<FNWorldAssemblyEditorCommands>
{
public:
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
