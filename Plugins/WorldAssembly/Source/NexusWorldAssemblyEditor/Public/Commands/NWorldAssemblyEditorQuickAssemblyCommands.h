// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class FBindingContext;
class FUICommandInfo;
class FUICommandList;

/**
 * The Quick Assembly commands of the World Assembly editor.
 *
 * The only category with no rail: these drive the level-editor toolbar button and its dropdown, so they are reached
 * from FNWorldAssemblyEditorToolMenu rather than from the edit mode's toolkit. The subsystem re-enters Start each
 * time the auto-assembly timer fires, which is why it is public rather than a bound handler.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/">World Assembly</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEditorQuickAssemblyCommands
{
public:
	/** @return The single instance holding this category's commands. */
	static FNWorldAssemblyEditorQuickAssemblyCommands& Get();

	/**
	 * Declare every Quick Assembly command against the module's binding context and map each to its action.
	 * @param Context The module's shared FBindingContext, supplied by FNWorldAssemblyEditorCommands::RegisterCommands.
	 */
	static void Register(const TSharedRef<FBindingContext>& Context);

	/** Toolbar click handler: cancels the tracked operation if one is running, otherwise starts a new one. */
	static void ButtonClicked();
	/** @return true when the button can act — always true while running (to allow cancel), else the start preconditions. */
	static bool Button_CanExecute();

	/** Start a Quick Assembly operation for the selected organ and track its ticket for the toggle. */
	static void Start();
	/** @return true if an operation can be started (valid organ, not PIE, no operation already running). */
	static bool Start_CanExecute();

	/** Cancel the tracked Quick Assembly operation if it is still running. */
	static void Cancel();

	/** Toggle UNWorldAssemblyEditorUserSettings::bQuickAssemblyLoadLevelInstances and persist it. */
	static void ToggleLoadInstances();
	/** @return checked state of the load-level-instances toggle for UI binding. */
	static bool ToggleLoadInstances_IsActionChecked();

	/** Toggle UNWorldAssemblyEditorUserSettings::bQuickAssemblyAutoAssembly and persist it. */
	static void ToggleAutoAssembly();
	/** @return checked state of the auto-assembly toggle for UI binding. */
	static bool ToggleAutoAssembly_IsActionChecked();

	/** The dropdown's two toggles, mapped to their actions. Handed to the FMenuBuilder that builds that menu. */
	TSharedPtr<FUICommandList> CommandList;

	TSharedPtr<FUICommandInfo> CommandInfo_ToggleLoadInstances;
	TSharedPtr<FUICommandInfo> CommandInfo_ToggleAutoAssembly;
};
