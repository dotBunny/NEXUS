// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

/**
 * Command callbacks and tool-menu registration for the NexusTooling editor module.
 *
 * Entries here power the "Tools" dropdowns, the project-levels sub-menu, and the blueprint-editor
 * right-click actions (e.g. "Open external documentation for this node").
 */
class FNToolingEditorCommands
{
public:
	/** Register every NexusTooling entry with the global tool-menu system. */
	static void AddMenuEntries();

	/** Remove every entry previously registered by AddMenuEntries. */
	static void RemoveMenuEntries();

	/** Spawn or focus the collision-visualizer editor-utility window. */
	static void CreateCollisionVisualizerWindow();

	/** @return true if the collision-visualizer window is currently open. */
	static bool HasCollisionVisualizerWindow();

	/** Open the engine's network profiler tab. */
	static void OpenNetworkProfiler();

	/** @return true if the network profiler tab is currently open. */
	static bool HasToolsProfileNetworkProfiler();

	/** Populate the per-project levels sub-menu from UNToolingEditorSettings::ProjectLevels. */
	static void GenerateProjectLevelsSubMenu(UToolMenu* Menu);

	/** Open the docs URL for the foreground blueprint node selection. */
	static void OnNodeExternalDocumentation();

	/** @return true if a blueprint node with an associated docs URL is currently selected. */
	static bool NodeExternalDocumentation_CanExecute();

};