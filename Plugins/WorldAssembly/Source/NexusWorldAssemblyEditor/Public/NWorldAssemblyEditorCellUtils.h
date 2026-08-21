// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

struct FNCellRootDetails;

/**
 * The author-time operations a cell command performs on the focused cell.
 *
 * Everything here mutates the cell actor, its root details, or its side-car package. FNCellEdModeRail
 * supplies the declaration, binding and gating; this supplies the work, so the cell editing behavior stays out of the
 * Slate layer entirely.
 *
 * @note Every entry point but AddActor assumes the focused world holds a cell actor. Callers gate on
 *       FNWorldAssemblyEditorUtils::CanEditCell before invoking, which is also what keeps these out of PIE. AddActor
 *       is the one that runs before there is a cell, so its own gate is the World category's
 *       FNWorldEdModeRail::AddCellActor_CanExecute, which asks the opposite question.
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class NEXUSWORLDASSEMBLYEDITOR_API FNWorldAssemblyEditorCellUtils
{
public:
	/** Place a new ANCellActor in the focused level, apply the project's default cell settings, and write its side-car. */
	static void AddActor();

	/** Remove the focused cell actor from the level and delete its side-car package. */
	static void RemoveActor();

	/** Reset the focused cell to its default authored state, clearing cached bounds, hull, voxel data and junctions. */
	static void ResetCell();

	/** Recompute bounds, hull, and voxel data for the focused cell. */
	static void CalculateAll();

	/** Recompute only the focused cell's axis-aligned bounds. */
	static void CalculateBounds();

	/** Recompute only the focused cell's convex hull. */
	static void CalculateHull();

	/** Recompute only the focused cell's voxel data. */
	static void CalculateVoxelData();

	/**
	 * Capture the active viewport as the thumbnail for both the focused level and its cell side-car.
	 * @note Draws the viewport three times — once per capture render mode, once to restore — and badges the side-car's
	 *       thumbnail afterwards. The level's thumbnail is left unbadged; it is an ordinary world asset.
	 */
	static void CaptureThumbnail();

	/**
	 * Run Mutator against the focused cell's root details inside a transaction, then mark the actor dirty.
	 *
	 * @param TransactionName Undo description shown for the change.
	 * @param Mutator Applied to the live root details; may touch any number of fields.
	 * @note No-ops when the focused world has no cell actor, so callers need no null handling of their own. Shared by
	 *       every Quick Options toggle, which are otherwise the same twelve lines of resolve/Modify/flip/dirty apiece.
	 */
	static void ModifyCellRootDetails(const FText& TransactionName, TFunctionRef<void(FNCellRootDetails&)> Mutator);

	/**
	 * @param Reader Applied to the focused cell's root details.
	 * @return Reader's result, or false when the focused world has no cell actor.
	 * @note The read half of ModifyCellRootDetails, backing the toggles' checked state. Evaluated on every UI tick, so
	 *       keep Reader to a field access.
	 */
	static bool QueryCellRootDetails(TFunctionRef<bool(const FNCellRootDetails&)> Reader);

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
};
