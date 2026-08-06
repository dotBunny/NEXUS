// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEdModeToolCommands.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

/**
 * Registers one tool-start command.
 * @note ToggleButton so the palette button stays lit while its tool is the active one.
 */
#define N_WORLDASSEMBLY_TOOL_COMMAND(CommandInfoSP, CommandName, CommandLabel, ToolTip, IconStyleName) \
	FUICommandInfo::MakeCommandInfo( \
		this->AsShared() \
		, CommandInfoSP \
		, CommandName \
		, CommandLabel \
		, ToolTip \
		, FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), IconStyleName) \
		, EUserInterfaceActionType::ToggleButton \
		, FInputChord())

void FNWorldAssemblyEdModeToolCommands::RegisterCommands()
{
	N_WORLDASSEMBLY_TOOL_COMMAND(BeginCellBoundsTool, TEXT("BeginCellBoundsTool"),
		LOCTEXT("CellBoundsToolLabel", "Bounds"),
		LOCTEXT("CellBoundsToolToolTip", "Drag the min and max corners of the cell's axis-aligned bounds."),
		"Command.WorldAssemblyEd.Bounds");

	N_WORLDASSEMBLY_TOOL_COMMAND(BeginCellHullTool, TEXT("BeginCellHullTool"),
		LOCTEXT("CellHullToolLabel", "Hull"),
		LOCTEXT("CellHullToolToolTip", "Drag the cell's hull vertices and split its edges (requires a tri-based hull)."),
		"Command.WorldAssemblyEd.Hull");

	N_WORLDASSEMBLY_TOOL_COMMAND(BeginCellVoxelTool, TEXT("BeginCellVoxelTool"),
		LOCTEXT("CellVoxelToolLabel", "Voxels"),
		LOCTEXT("CellVoxelToolToolTip", "Click voxels to toggle them between occupied and empty."),
		"Command.WorldAssemblyEd.Voxel.Points");

	N_WORLDASSEMBLY_TOOL_COMMAND(BeginJunctionPlacementTool, TEXT("BeginJunctionPlacementTool"),
		LOCTEXT("JunctionPlacementToolLabel", "Place"),
		LOCTEXT("JunctionPlacementToolToolTip", "Add a junction to the focused cell and position it."),
		"ClassIcon.NCellJunctionComponent");
}

#undef N_WORLDASSEMBLY_TOOL_COMMAND
#undef LOCTEXT_NAMESPACE
