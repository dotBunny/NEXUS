// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class FBindingContext;
class FUICommandInfo;
class FUICommandList;
class UNAssemblyOperation;
class UNOrganComponent;

/**
 * The Quick Assembly commands of the World Assembly editor, and the state they run against.
 *
 * The only category with no rail: these drive the level-editor toolbar button and its dropdown, so they are reached
 * from FNWorldAssemblyEditorToolMenu rather than from the edit mode's toolkit. The subsystem re-enters Start each
 * time the auto-assembly timer fires, which is why it is public rather than a bound handler.
 *
 * @note Owns the target organ, the tracked operation ticket and the toolbar progress fill, rather than leaving them
 *       on the tool menu that draws them. The menu, the style's icon attribute and the subsystem all read this;
 *       nothing here reads back out to the menu, which is what keeps the dependency one-way.
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

	/**
	 * @return The organ Quick Assembly acts on, or nullptr when the level holds none.
	 * @note Falls back to the level's first organ when nothing has been picked or the prior pick went stale, so the
	 *       button is usable without visiting the dropdown first.
	 */
	static UNOrganComponent* GetSelectedOrgan();

	/**
	 * Set the organ Quick Assembly acts on.
	 * @param OrganComponent The organ to target.
	 * @note Ignored while IsActive — the authoritative half of the selection lock the dropdown also enforces, so the
	 *       target cannot change out from under a running loop.
	 */
	static void SetSelectedOrgan(UNOrganComponent* OrganComponent);

	/** @return true if an organ is available to act on. */
	static bool HasValidOrgan();

	/** Set the toolbar progress-bar fill (clamped 0..1) and make it visible. Called from the assembly operation. */
	static void SetProgress(float InProgress);
	/** Hide the toolbar progress bar, e.g. when the operation finishes or is cancelled. */
	static void ClearProgress();
	/** @return Current progress fill, unset when no operation is in flight — which is what hides the bar. */
	static TOptional<float> GetProgress();

	/** Track Ticket as the operation this category owns; pass -1 to stop tracking. */
	static void SetOperationTicket(int32 Ticket);
	/** @return The tracked operation's ticket, or -1 when nothing is tracked. */
	static int32 GetOperationTicket();

	/** @return The tracked operation resolved from the ticket, or nullptr when there is none or it went stale. */
	static UNAssemblyOperation* GetTrackedOperation();

	/** @return true if the tracked operation exists and is still running. */
	static bool IsOperationRunning();

	/**
	 * @return true while a Quick Assembly loop is active — the tracked operation is running, or an auto-assembly
	 *         loop is waiting between runs.
	 * @note Drives the Start/Cancel toggle, the cancel icon, and the organ-selection lock.
	 */
	static bool IsActive();

	/** The dropdown's two toggles, mapped to their actions. Handed to the FMenuBuilder that builds that menu. */
	TSharedPtr<FUICommandList> CommandList;

	TSharedPtr<FUICommandInfo> CommandInfo_ToggleLoadInstances;
	TSharedPtr<FUICommandInfo> CommandInfo_ToggleAutoAssembly;
};
