// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class FBindingContext;
class FUICommandInfo;
class FUICommandList;

/**
 * The world-scoped commands of the World Assembly editor.
 *
 * The collision visualizer and the actor-ignore tagging toggle — the two things that act on the level as a whole
 * rather than on a cell, junction or organ. Both bodies are thin forwards, so this category has no operations
 * companion; the visualizer lives on UNWorldAssemblyEdMode and the tagging on FNWorldAssemblyEditorToolMenu.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEditorWorldCommands
{
public:
	/** @return The single instance holding this category's commands. */
	static FNWorldAssemblyEditorWorldCommands& Get();

	/**
	 * Declare every world command against the module's binding context and map each to its action.
	 * @param Context The module's shared FBindingContext, supplied by FNWorldAssemblyEditorCommands::RegisterCommands.
	 */
	static void Register(const TSharedRef<FBindingContext>& Context);

	/** Spawn the world-collision visualizer, or destroy it if one is already alive. */
	static void ToggleCollisionVisualizer();
	/** @return checked state of the collision-visualizer toggle for UI binding. */
	static bool ToggleCollisionVisualizer_IsActionChecked();

	/** Add or remove the world-collision-ignore tag across the current actor selection. */
	static void TagCollisionIgnore();
	/** @return true if the world-collision-ignore tagging action can act on the current selection. */
	static bool TagCollisionIgnore_CanExecute();

	/** Every world command, mapped to its action. Appended to the toolkit's list so the rails can resolve against it. */
	TSharedPtr<FUICommandList> CommandList;

	TSharedPtr<FUICommandInfo> CommandInfo_ToggleCollisionVisualizer;
	TSharedPtr<FUICommandInfo> CommandInfo_TagCollisionIgnore;
};
