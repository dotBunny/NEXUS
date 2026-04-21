// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NEditorStyleMacros.h"
#include "Templates/SharedPointer.h"
#include "Interfaces/IPluginManager.h"
#include "Brushes/SlateImageBrush.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

/**
 * Slate style set for the NexusProcGenEditor module.
 */
class FNProcGenEditorStyle
{
	N_IMPLEMENT_EDITOR_STYLE_HEADER("NexusProcGen", "NProcGenEditorStyle")

public:
	/** @return Icon for the "edit hull" mode action on the cell-actor toolbar. */
	static FSlateIcon CellActorEditHullModeIcon();

	/** @return Icon for the "edit bounds" mode action on the cell-actor toolbar. */
	static FSlateIcon CellActorEditBoundsModeIcon();

	/** @return Icon for the "edit voxel" mode action on the cell-actor toolbar. */
	static FSlateIcon CellActorEditVoxelModeIcon();

	/** @return Icon for the "toggle voxel-data overlay" action on the cell-actor toolbar. */
	static FSlateIcon CellActorToggleDrawVoxelDataIcon();
};