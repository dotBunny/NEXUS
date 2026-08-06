// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWorldAssemblyEditorStyle.h"

/**
 * The commands that start the World Assembly edit mode's interactive tools.
 *
 * Separate from FNWorldAssemblyEditorCommands (one-shot actions) and FNWorldAssemblyEdModePaletteCommands (the rail
 * categories) because these are handed to UEdMode::RegisterTool, which maps each one to the start/stop of a tool
 * rather than to a delegate of its own.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/editor-mode/">World Assembly Editor Mode</a>
 */
class FNWorldAssemblyEdModeToolCommands final : public TCommands<FNWorldAssemblyEdModeToolCommands>
{
public:
	FNWorldAssemblyEdModeToolCommands()
		: TCommands<FNWorldAssemblyEdModeToolCommands>(
			TEXT("NWorldAssemblyEdModeToolCommands"),
			NSLOCTEXT("NexusWorldAssemblyEditor", "NWorldAssemblyEdModeToolCommands", "World Assembly Editor Mode Tools"),
			NAME_None,
			FNWorldAssemblyEditorStyle::GetStyleSetName())
	{
	}

	//~TCommands
	virtual void RegisterCommands() override;
	//End TCommands

	/** Drag the focused cell's axis-aligned bounds by their min and max corners. */
	TSharedPtr<FUICommandInfo> BeginCellBoundsTool;

	/** Drag the focused cell's hull vertices, and split its edges. */
	TSharedPtr<FUICommandInfo> BeginCellHullTool;

	/** Toggle individual voxels of the focused cell between occupied and empty. */
	TSharedPtr<FUICommandInfo> BeginCellVoxelTool;

	/** Place and align junctions on the focused cell. */
	TSharedPtr<FUICommandInfo> BeginJunctionPlacementTool;
};

namespace NEXUS::WorldAssembly::Tools
{
	/** Tool identifiers passed to UEdMode::RegisterTool; also what ShouldToolStartBeAllowed and the active-tool checks key off. */
	inline const FString CellBounds = TEXT("NWorldAssembly.CellBoundsTool");
	inline const FString CellHull = TEXT("NWorldAssembly.CellHullTool");
	inline const FString CellVoxel = TEXT("NWorldAssembly.CellVoxelTool");
	inline const FString JunctionPlacement = TEXT("NWorldAssembly.JunctionPlacementTool");
}
