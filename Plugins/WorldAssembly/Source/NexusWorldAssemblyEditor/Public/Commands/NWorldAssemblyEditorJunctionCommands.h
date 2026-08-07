// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class FBindingContext;
class FUICommandInfo;
class FUICommandList;

/**
 * The junction-authoring commands of the World Assembly editor.
 *
 * Owns the declaration, binding and gating for everything that acts on the focused cell's junctions; the work lives
 * in FNWorldAssemblyEditorJunctionOperations.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEditorJunctionCommands
{
public:
	/** @return The single instance holding this category's commands. */
	static FNWorldAssemblyEditorJunctionCommands& Get();

	/**
	 * Declare every junction command against the module's binding context and map each to its action.
	 * @param Context The module's shared FBindingContext, supplied by FNWorldAssemblyEditorCommands::RegisterCommands.
	 */
	static void Register(const TSharedRef<FBindingContext>& Context);

	/** @return true when the focused level has a cell actor, actors are selected, and we are not in PIE. */
	static bool CanEditCellJunction();

	/** Every junction command, mapped to its action. Appended to the toolkit's list so the rails can resolve against it. */
	TSharedPtr<FUICommandList> CommandList;

	TSharedPtr<FUICommandInfo> CommandInfo_AddComponent;
	TSharedPtr<FUICommandInfo> CommandInfo_SelectComponent;
	TSharedPtr<FUICommandInfo> CommandInfo_CollectComponents;
};
