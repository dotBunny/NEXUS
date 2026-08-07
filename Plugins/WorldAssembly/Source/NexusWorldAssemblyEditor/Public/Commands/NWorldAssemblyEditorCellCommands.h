// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class FBindingContext;
class FUICommandInfo;
class FUICommandList;

/**
 * The cell-authoring commands of the World Assembly editor.
 *
 * Owns the declaration, binding and gating for everything that acts on the focused cell. The work itself lives in
 * FNWorldAssemblyEditorCellOperations, so this stays a description of what the UI offers rather than of what the
 * cell does. Declared against the module's shared binding context by FNWorldAssemblyEditorCommands, which is the
 * only TCommands in the module — these are plain classes so that every command keeps one Keyboard Shortcuts section.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEditorCellCommands
{
public:
	/** @return The single instance holding this category's commands. */
	static FNWorldAssemblyEditorCellCommands& Get();

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

	/** Select the cell actor that owns the current cell-editor context. */
	static void SelectActor();
	/** @return true if a cell-actor selection target is available. */
	static bool SelectActor_CanExecute();
	/** @return true if the select-actor entry should be shown in the current context. */
	static bool SelectActor_CanShow();

	/** @return true if voxel recalculation is possible (e.g. bounds exist). */
	static bool CalculateVoxelData_CanExecute();

	/** Save the focused cell to its side-car package. */
	static void SaveCell();

	/** Add or remove the cell-ignore tag across the current actor selection. */
	static void TagIgnore();
	/** @return true if the cell-ignore tagging action can act on the current selection. */
	static bool TagIgnore_CanExecute();

	/** @return true if the active viewport is suitable for thumbnail capture. */
	static bool CaptureThumbnail_CanExecute();

	/** Cycle the viewport overlay's voxel drawing through grid, points, and off. */
	static void ToggleDrawVoxelData();
	/** @return checked state of the draw-voxel-data toggle for UI binding. */
	static bool ToggleDrawVoxelData_IsActionChecked();

	/** Toggle whether the focused cell recalculates bounds automatically on world save. */
	static void ToggleBoundsCalculateOnSave();
	/** @return checked state of the bounds-calculate-on-save toggle for UI binding. */
	static bool ToggleBoundsCalculateOnSave_IsActionChecked();

	/** Toggle whether the focused cell recalculates the hull automatically on world save. */
	static void ToggleHullCalculateOnSave();
	/** @return checked state of the hull-calculate-on-save toggle for UI binding. */
	static bool ToggleHullCalculateOnSave_IsActionChecked();

	/** Toggle whether the focused cell's hull is allowed to be non-convex, clearing calculate-on-save when it is. */
	static void ToggleHullAllowNonConvex();
	/** @return checked state of the allow-non-convex-hull toggle for UI binding. */
	static bool ToggleHullAllowNonConvex_IsActionChecked();

	/** Toggle whether the focused cell recalculates voxel data automatically on world save. */
	static void ToggleVoxelCalculateOnSave();
	/** @return checked state of the voxel-calculate-on-save toggle for UI binding. */
	static bool ToggleVoxelCalculateOnSave_IsActionChecked();

	/** Toggle whether the focused cell stores voxel data at all, clearing what it holds when disabled. */
	static void ToggleVoxelData();
	/** @return checked state of the voxel-data-enabled toggle for UI binding. */
	static bool ToggleVoxelData_IsActionChecked();

	/** Every cell command, mapped to its action. Appended to the toolkit's list so the rails can resolve against it. */
	TSharedPtr<FUICommandList> CommandList;

	TSharedPtr<FUICommandInfo> CommandInfo_AddActor;
	TSharedPtr<FUICommandInfo> CommandInfo_SelectActor;
	TSharedPtr<FUICommandInfo> CommandInfo_RemoveActor;
	TSharedPtr<FUICommandInfo> CommandInfo_TagIgnore;
	TSharedPtr<FUICommandInfo> CommandInfo_CaptureThumbnail;

	TSharedPtr<FUICommandInfo> CommandInfo_CalculateAll;
	TSharedPtr<FUICommandInfo> CommandInfo_CalculateBounds;
	TSharedPtr<FUICommandInfo> CommandInfo_CalculateHull;
	TSharedPtr<FUICommandInfo> CommandInfo_CalculateVoxelData;

	TSharedPtr<FUICommandInfo> CommandInfo_ResetCell;
	TSharedPtr<FUICommandInfo> CommandInfo_SaveCell;

	TSharedPtr<FUICommandInfo> CommandInfo_ToggleDrawVoxelData;
	TSharedPtr<FUICommandInfo> CommandInfo_ToggleBoundsCalculateOnSave;
	TSharedPtr<FUICommandInfo> CommandInfo_ToggleHullCalculateOnSave;
	TSharedPtr<FUICommandInfo> CommandInfo_ToggleHullAllowNonConvex;
	TSharedPtr<FUICommandInfo> CommandInfo_ToggleVoxelCalculateOnSave;
	TSharedPtr<FUICommandInfo> CommandInfo_ToggleVoxelData;
};
