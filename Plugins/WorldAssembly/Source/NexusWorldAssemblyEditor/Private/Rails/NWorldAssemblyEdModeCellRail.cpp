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
					Commands.CommandInfo_CellCalculateAll,
					Commands.CommandInfo_CellCalculateBounds,
					Commands.CommandInfo_CellCalculateHull,
					//Commands.CommandInfo_CellCalculateVoxelData,
				})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandList(
				LOCTEXT("CellHeader_Tagging", "Tagging"),
				{
					Commands.CommandInfo_CellTagIgnore
				})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandList(
				LOCTEXT("CellHeader_Actions", "Actions"),
				{
					Commands.CommandInfo_CellSelectActor,
					//Commands.CommandInfo_CellToggleDrawVoxelData,
					Commands.CommandInfo_CellCaptureThumbnail
				})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCheckList(
				LOCTEXT("CellHeader_QuickOptions", "Quick Options"),
				{
					Commands.CommandInfo_CellToggleBoundsCalculateOnSave,
					Commands.CommandInfo_CellToggleHullCalculateOnSave,
					Commands.CommandInfo_CellToggleHullAllowNonConvex,
					//Commands.CommandInfo_CellToggleVoxelCalculateOnSave,
					//Commands.CommandInfo_CellToggleVoxelData,
				})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandGrid(
				LOCTEXT("CellHeader_CellData", "Cell Data"),
				{
					Commands.CommandInfo_CellSaveCell,
					Commands.CommandInfo_CellResetCell,
					Commands.CommandInfo_CellRemoveActor
				})
		];
}

#undef LOCTEXT_NAMESPACE
