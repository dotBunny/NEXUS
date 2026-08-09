// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class FBindingContext;
class FUICommandInfo;
class FUICommandList;

/**
 * The cell data-authoring commands of the World Assembly editor.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEditorCellDataCommands
{
public:
	/** @return The single instance holding this category's commands. */
	static FNWorldAssemblyEditorCellDataCommands& Get();

	/**
	 * Declare every cell command against the module's binding context and map each to its action.
	 * @param Context The module's shared FBindingContext, supplied by FNWorldAssemblyEditorCommands::RegisterCommands.
	 */
	static void Register(const TSharedRef<FBindingContext>& Context);

	/**
	 * @return true when the focused level has a cell actor to act on and we are not in PIE.
	 * @note The default gate for cell commands. Every one of these mutates or saves cell data, which is authoring
	 *       work with no meaning against a play world — the toolkit panel stays up during PIE, so without this they
	 *       would happily run against it. Interactive tools get the same protection from
	 *       UEdMode::ShouldToolStartBeAllowed; commands have no such default, so it is stated here.
	 */
	static bool CanEditCell();

	/** @return true if a cell actor can be added: the level has none already, and holds no organs. */
	static bool AddActor_CanExecute();

	/** Save the focused cell to its side-car package. */
	static void SaveCell();

	/** Add or remove the cell-ignore tag across the current actor selection. */
	static void TagIgnore();
	/** @return true if the cell-ignore tagging action can act on the current selection. */
	static bool TagIgnore_CanExecute();

	/** @return true if the active viewport is suitable for thumbnail capture. */
	static bool CaptureThumbnail_CanExecute();

	/** Every cell command, mapped to its action. Appended to the toolkit's list so the rails can resolve against it. */
	TSharedPtr<FUICommandList> CommandList;

	TSharedPtr<FUICommandInfo> CommandInfo_AddActor;
	TSharedPtr<FUICommandInfo> CommandInfo_RemoveActor;

	TSharedPtr<FUICommandInfo> CommandInfo_CaptureThumbnail;

	TSharedPtr<FUICommandInfo> CommandInfo_ResetCell;
	TSharedPtr<FUICommandInfo> CommandInfo_SaveCell;

};
