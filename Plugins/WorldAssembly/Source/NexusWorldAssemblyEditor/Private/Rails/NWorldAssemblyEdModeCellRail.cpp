// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Rails/NWorldAssemblyEdModeCellRail.h"

#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblyEdModePaletteCommands.h"
#include "NWorldAssemblyEdModeToolCommands.h"
#include "Commands/NWorldAssemblyEditorCellCommands.h"
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
	const FNWorldAssemblyEditorCellCommands& Cell = FNWorldAssemblyEditorCellCommands::Get();
	const FNWorldAssemblyEdModeToolCommands& ToolCommands = FNWorldAssemblyEdModeToolCommands::Get();

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandPalette(
				LOCTEXT("CellHeader_Tools", "Tools"),
				{
					ToolCommands.BeginCellBoundsTool,
					ToolCommands.BeginCellHullVertexTool,
					ToolCommands.BeginCellHullSplitTool,
					// We don't use Voxels right now so were going to hide the tooling
					// ToolCommands.BeginCellVoxelTool,
				})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandGrid(
				LOCTEXT("CellHeader_Calculate", "Calculate"),
				{
					Cell.CommandInfo_CalculateAll,
					Cell.CommandInfo_CalculateBounds,
					Cell.CommandInfo_CalculateHull,
					//Cell.CommandInfo_CalculateVoxelData,
				})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandList(
				LOCTEXT("CellHeader_Tagging", "Tagging"),
				{
					Cell.CommandInfo_TagIgnore
				})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandList(
				LOCTEXT("CellHeader_Actions", "Actions"),
				{
					Cell.CommandInfo_SelectActor,
					//Cell.CommandInfo_ToggleDrawVoxelData,
					Cell.CommandInfo_CaptureThumbnail
				})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCheckList(
				LOCTEXT("CellHeader_QuickOptions", "Quick Options"),
				{
					Cell.CommandInfo_ToggleBoundsCalculateOnSave,
					Cell.CommandInfo_ToggleHullCalculateOnSave,
					Cell.CommandInfo_ToggleHullAllowNonConvex,
					//Cell.CommandInfo_ToggleVoxelCalculateOnSave,
					//Cell.CommandInfo_ToggleVoxelData,
				})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandGrid(
				LOCTEXT("CellHeader_CellData", "Cell Data"),
				{
					Cell.CommandInfo_SaveCell,
					Cell.CommandInfo_ResetCell,
					Cell.CommandInfo_RemoveActor
				})
		];
}

#undef LOCTEXT_NAMESPACE
