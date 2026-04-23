// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Registers every ProcGen entry on the editor's tool menus — cell/organ/junction dropdowns,
 * toolbar buttons, and the editor-utility-window launcher.
 */
class FNProcGenEditorToolMenu
{
public:
	/** Register all ProcGen entries with the global tool-menu system. */
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
	 * FNProcGenEdMode::CreateCollisionVisualizer when none is alive, otherwise destroys the existing one.
	 */
	static void CollisionVisualizerToggle();

	/** Spawn or focus the EUW-based ProcGen tool window. */
	static void CreateEditorUtilityWindow();

	/** @return true if the ProcGen tool window is currently open. */
	static bool HasEditorUtilityWindow();
};
