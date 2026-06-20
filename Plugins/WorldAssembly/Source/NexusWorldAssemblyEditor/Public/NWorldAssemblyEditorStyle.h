// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NEditorStyleMacros.h"
#include "Textures/SlateIcon.h" // FSlateIcon return types declared below

/**
 * Slate style set for the NexusWorldAssemblyEditor module.
 */
class FNWorldAssemblyEditorStyle
{
	N_EDITOR_STYLE_HEADER("NexusWorldAssembly", "NWorldAssemblyEditorStyle")

public:
	/** @return Icon for the "edit hull" mode action on the cell-actor toolbar. */
	static FSlateIcon CellActorEditHullModeIcon();

	/** @return Icon for the "edit bounds" mode action on the cell-actor toolbar. */
	static FSlateIcon CellActorEditBoundsModeIcon();

	/** @return Icon for the "edit voxel" mode action on the cell-actor toolbar. */
	static FSlateIcon CellActorEditVoxelModeIcon();

	/** @return Icon for the "toggle voxel-data overlay" action on the cell-actor toolbar. */
	static FSlateIcon CellActorToggleDrawVoxelDataIcon();

	/** @return Icon for the "toggle world-collision visualizer" action on the World Assembly toolbar. */
	static FSlateIcon CollisionVisualizerToggleIcon();

	/** @return Icon for the cell-ignore tagging action. */
	static FSlateIcon CellIgnoreIcon();
	/** @return Icon for the world-collision-ignore tagging action. */
	static FSlateIcon WorldCollisionIgnoreIcon();

	/** @return Cancel icon while the tracked Quick Assembly operation is running, otherwise the start icon. */
	static FSlateIcon QuickAssemblyOperationIcon();
};