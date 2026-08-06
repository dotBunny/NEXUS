// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Rails/NWorldAssemblyEdModeCellRail.h"

#include "NWorldAssemblyEditorCommands.h"
#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblyEdModePaletteCommands.h"
#include "NWorldAssemblyEdModeToolCommands.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

TSharedPtr<FUICommandInfo> FNWorldAssemblyEdModeCellRail::GetCategoryCommand() const
{
	return FNWorldAssemblyEdModePaletteCommands::Get().LoadCellPalette;
}

TAttribute<bool> FNWorldAssemblyEdModeCellRail::GetEnabled() const
{
	return TAttribute<bool>::CreateLambda(&UNWorldAssemblyEdMode::HasCellActor);
}

TSharedPtr<SWidget> FNWorldAssemblyEdModeCellRail::CreateContent() const
{
	const FNWorldAssemblyEditorCommands& Commands = FNWorldAssemblyEditorCommands::Get();
	const FNWorldAssemblyEdModeToolCommands& ToolCommands = FNWorldAssemblyEdModeToolCommands::Get();

	return SNew(SVerticalBox)

		// The interactive tools: enter one and edit that aspect of the cell in the viewport.
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandList(
				LOCTEXT("CellHeader_Tools", "Tools"),
				{
					ToolCommands.BeginCellBoundsTool,
					ToolCommands.BeginCellHullTool,
					ToolCommands.BeginCellVoxelTool,
				})
		]

		// Persistent per-cell settings rather than actions, so checkboxes: each one reads as on or off at a glance
		// and stays that way until changed.
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCheckList(
				LOCTEXT("CellHeader_QuickOptions", "Quick Options"),
				{
					Commands.CommandInfo_CellToggleBoundsCalculateOnSave,
					Commands.CommandInfo_CellToggleHullAllowNonConvex,
					Commands.CommandInfo_CellToggleHullCalculateOnSave,
					Commands.CommandInfo_CellToggleVoxelCalculateOnSave,
					Commands.CommandInfo_CellToggleVoxelData,
				})
		]

		// One-shot operations on the focused cell.
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandList(
				LOCTEXT("CellHeader_Actions", "Actions"),
				{
					Commands.CommandInfo_CellSelectActor,
					Commands.CommandInfo_CellCalculateAll,
					Commands.CommandInfo_CellCalculateBounds,
					Commands.CommandInfo_CellCalculateHull,
					Commands.CommandInfo_CellCalculateVoxelData,
					Commands.CommandInfo_CellToggleDrawVoxelData,
					Commands.CommandInfo_CellHullSplitEdge,
					Commands.CommandInfo_CellTagIgnore,
					Commands.CommandInfo_CellCaptureThumbnail,
					Commands.CommandInfo_CellSaveCell,
					Commands.CommandInfo_CellResetCell,
					Commands.CommandInfo_CellRemoveActor,
				})
		];
}

#undef LOCTEXT_NAMESPACE
