// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Rails/NWorldAssemblyEdModeCellRail.h"

#include "NWorldAssemblyEditorUtils.h"
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

TAttribute<bool> FNWorldAssemblyEdModeCellRail::GetAvailable() const
{
	// Level contents rather than the focused actor. The category is worth showing whenever the level has a cell to
	// author, whether or not one is selected this instant; the buttons inside it are what go dead when nothing is
	// focused.
	return TAttribute<bool>::CreateStatic(&FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld);
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
					Cell.CommandInfo_CalculateBounds,
					Cell.CommandInfo_CalculateHull,
					Cell.CommandInfo_CalculateAll,
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
		];
}

#undef LOCTEXT_NAMESPACE
