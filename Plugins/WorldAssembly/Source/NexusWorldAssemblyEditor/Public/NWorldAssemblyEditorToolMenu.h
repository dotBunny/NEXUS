// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Organ/NOrganComponent.h"

class UNAssemblyOperation;

/**
 * Registers every World Assembly entry on the editor's tool menus — cell/organ/junction dropdowns,
 * toolbar buttons, and the editor-utility-window launcher.
 */
class FNWorldAssemblyEditorToolMenu
{
public:

	/** Name of the global tool-menu section World Assembly entries are added to. */
	static const FName MenuSectionGlobal;
	/** Name of the World Assembly-specific tool-menu section. */
	static const FName MenuSection;

	/** Register all World Assembly entries with the global tool-menu system. */
	static void AddMenuEntries();

	/** Remove every entry previously registered by AddMenuEntries. */
	static void RemoveMenuEntries();

	/** @return true if the cell-edit-mode action should be visible given the current context. */
	static bool ShowCellEditMode();

	/** @return true if the draw-voxels action should be visible given the current context. */
	static bool ShowDrawVoxels();

	/** @return true if the cell-actor dropdown should be visible given the current context. */
	static bool ShowCellDropdown();

	/** @return true if the cell-junction dropdown should be visible given the current context. */
	static bool ShowCellJunctionDropdown();

	/** @return true if the organ dropdown should be visible given the current context. */
	static bool ShowOrganDropdown();

	/**
	 * Toggles the world-collision visualizer for the currently-edited world. Spawns a fresh visualizer via
	 * FNWorldAssemblyEdMode::CreateCollisionVisualizer when none is alive, otherwise destroys the existing one.
	 */
	static void CollisionVisualizerToggle();

	/**
	 * Adds or removes the cell-ignore tag on the current actor selection. The direction is decided by
	 * GetIgnoreSelectedActorsToggleMode: if no actor in the selection is tagged the tag is added to all,
	 * otherwise the tag is removed from all.
	 */
	static void TagSelectedActors_CellIgnore();

	/** @return true if the cell-ignore tagging action should be shown for the current selection. */
	static bool TagSelectedActors_CellIgnore_CanShow();

	/**
	 * @return Toggle state for the ignore-selected-actors action: 0 to add the tag, 1 to remove it, or -1
	 * when the action should be disabled (empty selection or a mixed-tag selection where the intent is ambiguous).
	 */
	static int32 TagSelectedActors_CellIgnore_Mode();

	/** Adds or removes the world-collision-ignore tag on the current actor selection (direction decided by TagSelectedActors_WorldIgnore_Mode). */
	static void TagSelectedActors_WorldCollisionIgnore();

	/** @return true if the world-collision-ignore tagging action should be shown for the current selection. */
	static bool TagSelectedActors_WorldCollisionIgnore_CanShow();

	/** @return Toggle state for the world-collision-ignore action: 0 to add the tag, 1 to remove it, or -1 when it should be disabled. */
	static int32 TagSelectedActors_WorldIgnore_Mode();


	/** Splits the currently-selected hull edge on the active cell. */
	static void Hull_SplitEdge();
	/** @return true if the hull split-edge action should be shown (an editable hull edge is selected). */
	static bool Hull_SplitEdge_CanShow();

	static bool ShowQuickAssembly();
	static bool HasValidQuickAssemblyOrgan();
	static TSharedRef<SWidget> CreateQuickAssemblyComboBox();

	/** @return A numeric-entry widget bound to UNWorldAssemblyEditorUserSettings::QuickAssemblyAutoAssemblyTimer (seconds), for the Quick Assembly options dropdown. */
	static TSharedRef<SWidget> CreateQuickAssemblyAutoAssemblyTimerWidget();

	static void SetSelectedQuickAssemblyOption(UNOrganComponent* OrganComponent);
	static UNOrganComponent* GetQuickAssemblyOrganComponent();

	/** Sets the Quick Assembly progress bar fill (clamped 0..1) and makes it visible. Call from the assembly operation. */
	static void SetQuickAssemblyProgress(float InProgress);
	/** Hides the Quick Assembly progress bar (e.g. when the operation finishes or is cancelled). */
	static void ClearQuickAssemblyProgress();
	static void SetQuickAssemblyOperationTicket(int32 Ticket) { QuickAssemblyOperationTicket = Ticket; }
	static int32 GetQuickAssemblyOperationTicket() { return QuickAssemblyOperationTicket; }

	/** @return The tracked Quick Assembly operation resolved from QuickAssemblyOperationTicket, or nullptr if none/stale. */
	static UNAssemblyOperation* GetTrackedQuickAssemblyOperation();
	/** @return true if the tracked Quick Assembly operation exists and is still running (drives the Start/Cancel toggle). */
	static bool IsQuickAssemblyOperationRunning();

	/**
	 * @return true while a Quick Assembly loop is active — the tracked operation is running, or an auto-assembly
	 * loop is waiting between runs. Drives the Start/Cancel toggle, the cancel icon, and the organ-selection lock.
	 */
	static bool IsQuickAssemblyActive();

private:
	static TWeakObjectPtr<UNOrganComponent> QuickAssemblyOrganComponent;
	static int32 QuickAssemblyOperationTicket;
	/** Current Quick Assembly progress (0..1). Unset when no operation is in flight, which hides the bar. */
	static TOptional<float> QuickAssemblyProgress;
};
