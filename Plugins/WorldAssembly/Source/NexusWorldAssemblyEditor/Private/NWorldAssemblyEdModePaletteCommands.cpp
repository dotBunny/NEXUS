// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEdModePaletteCommands.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

/**
 * Registers one rail category command.
 * @note ToggleButton rather than Button: the rail shows which category is selected, so the command carries a checked
 *       state that FToolkitBuilder drives off the active palette.
 */
#define N_WORLDASSEMBLY_PALETTE_COMMAND(CommandInfoSP, CommandName, CommandLabel, ToolTip, IconStyleName) \
	FUICommandInfo::MakeCommandInfo( \
		this->AsShared() \
		, CommandInfoSP \
		, CommandName \
		, CommandLabel \
		, ToolTip \
		, FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), IconStyleName) \
		, EUserInterfaceActionType::ToggleButton \
		, FInputChord())

void FNWorldAssemblyEdModePaletteCommands::RegisterCommands()
{
	N_WORLDASSEMBLY_PALETTE_COMMAND(LoadCellPalette, TEXT("EnterCellPalette"),
		LOCTEXT("CellPaletteLabel", "Cell"),
		LOCTEXT("CellPaletteToolTip", "Author the focused cell: edit its bounds, hull and voxel data, and manage its side-car asset."),
		"ClassIcon.NCellActor");

	N_WORLDASSEMBLY_PALETTE_COMMAND(LoadJunctionPalette, TEXT("EnterJunctionPalette"),
		LOCTEXT("JunctionPaletteLabel", "Junction"),
		LOCTEXT("JunctionPaletteToolTip", "Place and align the focused cell's junctions."),
		"ClassIcon.NCellJunctionComponent");

	// The .Small variant deliberately: the other three categories are 16x16 class icons, and the plain
	// Icon.WorldAssembly brush is the 128x128 one the Modes dropdown uses, which stretches the rail button to match.
	N_WORLDASSEMBLY_PALETTE_COMMAND(LoadWorldPalette, TEXT("EnterWorldPalette"),
		LOCTEXT("WorldPaletteLabel", "World"),
		LOCTEXT("WorldPaletteToolTip", "World-scoped tools: the collision visualizer and the actor-ignore tagging toggles."),
		"Icon.WorldAssembly.Small");

	N_WORLDASSEMBLY_PALETTE_COMMAND(LoadOrganPalette, TEXT("EnterOrganPalette"),
		LOCTEXT("OrganPaletteLabel", "Organ"),
		LOCTEXT("OrganPaletteToolTip", "Generate and clear organ proxies, and manage their level instances."),
		"ClassIcon.NOrganComponent");
}

#undef N_WORLDASSEMBLY_PALETTE_COMMAND
#undef LOCTEXT_NAMESPACE
