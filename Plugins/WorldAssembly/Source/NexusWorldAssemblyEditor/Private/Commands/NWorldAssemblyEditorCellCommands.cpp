// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Commands/NWorldAssemblyEditorCellCommands.h"

#include "Editor.h"
#include "NEditorUtils.h"
#include "NUIEditorStyle.h"
#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblyEditorStyle.h"
#include "NWorldAssemblyEditorUtils.h"
#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblyUtils.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellRootDetails.h"
#include "Cell/NCellVoxelData.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/UICommandList.h"
#include "Operations/NWorldAssemblyEditorCellOperations.h"
#include "Operations/NWorldAssemblyEditorTagOperations.h"
#include "Styling/AppStyle.h"

FNWorldAssemblyEditorCellCommands& FNWorldAssemblyEditorCellCommands::Get()
{
	static FNWorldAssemblyEditorCellCommands Instance;
	return Instance;
}

void FNWorldAssemblyEditorCellCommands::Register(const TSharedRef<FBindingContext>& Context)
{
	FNWorldAssemblyEditorCellCommands& Commands = Get();

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_CalculateAll,
		"NWorldAssembly.NCell.CalculateAll",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateAll", "All"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateAll_Tooltip", "Calculate all data related to the cell."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_CalculateBounds,
		"NWorldAssembly.NCell.CalculateBounds",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateBounds", "Bounds"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateBounds_Tooltip", "Calculate bounds for the cell."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.CalculateBounds"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_CalculateHull,
		"NWorldAssembly.NCell.CalculateHull",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateHull", "Hull"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateHull_Tooltip", "Calculate convex hull for the cell."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.CalculateHull"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_CalculateVoxelData,
		"NWorldAssembly.NCell.CalculateVoxelData",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateVoxelData", "Voxel"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateVoxelData_Tooltip", "Calculate voxel data for the cell."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.CalculateVoxelData"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_ToggleBoundsCalculateOnSave,
		"NWorldAssembly.NCell.ToggleBoundsCalculateOnSave",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleBoundsCalculateOnSave", "Calculate Bounds On Save"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleBoundsCalculateOnSave_Tooltip", "Calculates the bounds of the cell when the level is saved."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.CalculateBounds"),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_ToggleHullAllowNonConvex,
		"NWorldAssembly.NCell.ToggleHullAllowNonConvex",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleHullAllowNonConvex", "Allow Non-Convex Hull"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleHullAllowNonConvex_Tooltip", "Allows for more complex collision mesh to be used instead of optimized convex hull."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.Hull.AllowNonConvex"),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_ToggleHullCalculateOnSave,
		"NWorldAssembly.NCell.ToggleHullCalculateOnSave",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleHullCalculateOnSave", "Calculate Hull On Save"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleHullCalculateOnSave_Tooltip", "Calculates the hull of the cell when the level is saved."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.CalculateHull"),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_ToggleVoxelCalculateOnSave,
		"NWorldAssembly.NCell.ToggleVoxelCalculateOnSave",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleVoxelCalculateOnSave", "Calculate Voxel Data On Save"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleVoxelCalculateOnSave_Tooltip", "Calculates the voxel data of the cell when the level is saved."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.CalculateVoxelData"),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_ToggleVoxelData,
		"NWorldAssembly.NCell.ToggleVoxelData",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleVoxelData", "Use Voxel Data w/ Cell"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleVoxelData_Tooltip", "Should voxel data be generated and associated to this cell?"),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.Voxel.Grid.Selected"),
		EUserInterfaceActionType::ToggleButton, FInputChord());



	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_SelectActor,
		"NWorldAssembly.NCell.SelectActor",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_SelectActor", "Select Actor"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_SelectActor_Tooltip", "Select the NCellActor in the level."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.SelectNCellActor.Selected"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_ToggleDrawVoxelData,
		"NWorldAssembly.NCell.ToggleDrawVoxelData",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_DrawVoxelData", "Draw Voxel Data"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_DrawVoxelData_Tooltip", "Toggle drawing calculated voxel data for the NCell."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.Voxel.Points"),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	// Cell-scoped despite acting on the actor selection: the tag it toggles only means anything to a cell's
	// bounds/hull/voxel calculations, which is why it sits with the Cell actions rather than the world ones.
	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_TagIgnore,
		"NWorldAssembly.NCell.TagIgnore",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_TagIgnore", "Ignore Cell Collision"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_TagIgnore_Tooltip", "Toggles the necessary tag to have the selected actors ignored when calculating the bounds/hull/etc for a Cell."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.CellIgnore_NotIgnored"),
		EUserInterfaceActionType::Button, FInputChord());

	Commands.CommandList = MakeShared<FUICommandList>();



	Commands.CommandList->MapAction(Commands.CommandInfo_SelectActor,
		FExecuteAction::CreateStatic(&SelectActor),
		FCanExecuteAction::CreateStatic(&SelectActor_CanExecute));

	Commands.CommandList->MapAction(Commands.CommandInfo_ToggleDrawVoxelData,
		FExecuteAction::CreateStatic(&ToggleDrawVoxelData),
		FCanExecuteAction::CreateStatic(&CanEditCell),
		FIsActionChecked::CreateStatic(&ToggleDrawVoxelData_IsActionChecked));

	Commands.CommandList->MapAction(Commands.CommandInfo_TagIgnore,
		FExecuteAction::CreateStatic(&TagIgnore),
		FCanExecuteAction::CreateStatic(&TagIgnore_CanExecute));

	Commands.CommandList->MapAction(Commands.CommandInfo_CalculateAll,
		FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCellOperations::CalculateAll),
		FCanExecuteAction::CreateStatic(&Calculate_CanExecute));

	Commands.CommandList->MapAction(Commands.CommandInfo_CalculateBounds,
		FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCellOperations::CalculateBounds),
		FCanExecuteAction::CreateStatic(&Calculate_CanExecute));

	Commands.CommandList->MapAction(Commands.CommandInfo_CalculateHull,
		FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCellOperations::CalculateHull),
		FCanExecuteAction::CreateStatic(&Calculate_CanExecute));

	Commands.CommandList->MapAction(Commands.CommandInfo_CalculateVoxelData,
		FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCellOperations::CalculateVoxelData),
		FCanExecuteAction::CreateStatic(&CalculateVoxelData_CanExecute));

	Commands.CommandList->MapAction(Commands.CommandInfo_ToggleBoundsCalculateOnSave,
		FExecuteAction::CreateStatic(&ToggleBoundsCalculateOnSave),
		FCanExecuteAction::CreateStatic(&CanEditCell),
		FIsActionChecked::CreateStatic(&ToggleBoundsCalculateOnSave_IsActionChecked));

	Commands.CommandList->MapAction(Commands.CommandInfo_ToggleHullCalculateOnSave,
		FExecuteAction::CreateStatic(&ToggleHullCalculateOnSave),
		FCanExecuteAction::CreateStatic(&CanEditCell),
		FIsActionChecked::CreateStatic(&ToggleHullCalculateOnSave_IsActionChecked));

	Commands.CommandList->MapAction(Commands.CommandInfo_ToggleHullAllowNonConvex,
		FExecuteAction::CreateStatic(&ToggleHullAllowNonConvex),
		FCanExecuteAction::CreateStatic(&CanEditCell),
		FIsActionChecked::CreateStatic(&ToggleHullAllowNonConvex_IsActionChecked));

	Commands.CommandList->MapAction(Commands.CommandInfo_ToggleVoxelCalculateOnSave,
		FExecuteAction::CreateStatic(&ToggleVoxelCalculateOnSave),
		FCanExecuteAction::CreateStatic(&CanEditCell),
		FIsActionChecked::CreateStatic(&ToggleVoxelCalculateOnSave_IsActionChecked));

	Commands.CommandList->MapAction(Commands.CommandInfo_ToggleVoxelData,
		FExecuteAction::CreateStatic(&ToggleVoxelData),
		FCanExecuteAction::CreateStatic(&CanEditCell),
		FIsActionChecked::CreateStatic(&ToggleVoxelData_IsActionChecked));
}

bool FNWorldAssemblyEditorCellCommands::CanEditCell()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;
	return FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld();
}

void FNWorldAssemblyEditorCellCommands::SelectActor()
{
	GEditor->SelectNone(false, true);
	GEditor->SelectActor(FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld(),
		true, true, true, true);
}

bool FNWorldAssemblyEditorCellCommands::SelectActor_CanExecute()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;
	return UNWorldAssemblyEdMode::HasCellActor() && GEditor->CanSelectActor(UNWorldAssemblyEdMode::GetCellActor(), false);
}

bool FNWorldAssemblyEditorCellCommands::SelectActor_CanShow()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	// Use cached version
	return UNWorldAssemblyEdMode::IsActive() && UNWorldAssemblyEdMode::HasCellActor() && !FNWorldAssemblyEditorUtils::IsCellActorSelected();
}

bool FNWorldAssemblyEditorCellCommands::Calculate_CanExecute()
{
	return CanEditCell() && UNWorldAssemblyEdMode::IsTerrainSettled();
}

bool FNWorldAssemblyEditorCellCommands::CalculateVoxelData_CanExecute()
{
	if (!Calculate_CanExecute()) return false;

	const ANCellActor* CellActor = FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld();
	if (CellActor == nullptr) return false;
	return CellActor->GetCellRoot()->Details.VoxelSettings.bUseVoxelData;
}

void FNWorldAssemblyEditorCellCommands::SaveCell()
{
	UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	ANCellActor* CellActor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);

	// Here rather than inside SaveCell: this is the last point before the engine's save machinery takes over, and the
	// wait must not run once it has — see WaitForTerrainToSettle's remark.
	if (CellActor != nullptr)
	{
		FNWorldAssemblyEditorUtils::WaitForTerrainToSettle(CellActor->GetLevel());
	}

	FNWorldAssemblyEditorUtils::SaveCell(CurrentWorld, CellActor, true);
}

void FNWorldAssemblyEditorCellCommands::TagIgnore()
{
	FNWorldAssemblyEditorTagOperations::ToggleTagOnSelection(
		NEXUS::WorldAssembly::ActorTags::CellIgnore,
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCellCommands_TagIgnore_Add", "Add CellIgnore Tags"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCellCommands_TagIgnore_Remove", "Remove CellIgnore Tags"));
}

bool FNWorldAssemblyEditorCellCommands::TagIgnore_CanExecute()
{
	// Tagging writes to the actors and opens a transaction, so it is authoring work like the rest of the cell
	// commands — not something to run against a play world.
	if (FNEditorUtils::IsPlayInEditor()) return false;

	return UNWorldAssemblyEdMode::HasCellActor()
		&& FNEditorUtils::HasActorsSelected()
		&& !FNWorldAssemblyEditorUtils::IsCellActorSelected();
}

bool FNWorldAssemblyEditorCellCommands::CaptureThumbnail_CanExecute()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	const UWorld* World = FNEditorUtils::GetCurrentWorld();
	return World != nullptr && !FNEditorUtils::IsUnsavedWorld(World);
}

void FNWorldAssemblyEditorCellCommands::ToggleDrawVoxelData()
{
	switch (UNWorldAssemblyEdMode::GetCellVoxelMode())
	{
		using enum UNWorldAssemblyEdMode::ENCellVoxelMode;
	case Grid:
		UNWorldAssemblyEdMode::SetCellVoxelMode(Points);
		break;
	case None:
		UNWorldAssemblyEdMode::SetCellVoxelMode(Grid);
		break;
	default:
		UNWorldAssemblyEdMode::SetCellVoxelMode(None);
	}
}

bool FNWorldAssemblyEditorCellCommands::ToggleDrawVoxelData_IsActionChecked()
{
	return UNWorldAssemblyEdMode::GetCellVoxelMode() != UNWorldAssemblyEdMode::ENCellVoxelMode::None;
}

void FNWorldAssemblyEditorCellCommands::ToggleBoundsCalculateOnSave()
{
	FNWorldAssemblyEditorCellOperations::ModifyCellRootDetails(
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCellCommands_ToggleBoundsCalculateOnSave", "Toggle Calculate Bounds On Save"),
		[](FNCellRootDetails& Details)
		{
			Details.BoundsSettings.bCalculateOnSave = !Details.BoundsSettings.bCalculateOnSave;
		});
}

bool FNWorldAssemblyEditorCellCommands::ToggleBoundsCalculateOnSave_IsActionChecked()
{
	return FNWorldAssemblyEditorCellOperations::QueryCellRootDetails(
		[](const FNCellRootDetails& Details) { return Details.BoundsSettings.bCalculateOnSave; });
}

void FNWorldAssemblyEditorCellCommands::ToggleHullCalculateOnSave()
{
	FNWorldAssemblyEditorCellOperations::ModifyCellRootDetails(
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCellCommands_ToggleHullCalculateOnSave", "Toggle Calculate Hull On Save"),
		[](FNCellRootDetails& Details)
		{
			Details.HullSettings.bCalculateOnSave = !Details.HullSettings.bCalculateOnSave;
		});
}

bool FNWorldAssemblyEditorCellCommands::ToggleHullCalculateOnSave_IsActionChecked()
{
	return FNWorldAssemblyEditorCellOperations::QueryCellRootDetails(
		[](const FNCellRootDetails& Details) { return Details.HullSettings.bCalculateOnSave; });
}

void FNWorldAssemblyEditorCellCommands::ToggleHullAllowNonConvex()
{
	FNWorldAssemblyEditorCellOperations::ModifyCellRootDetails(
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCellCommands_ToggleHullAllowNonConvex", "Toggle Allow Non-Convex Hull"),
		[](FNCellRootDetails& Details)
		{
			Details.HullSettings.bAllowNonConvex = !Details.HullSettings.bAllowNonConvex;

			// A non-convex hull is authored by hand, so the on-save recalculation that would flatten it back to a
			// convex one is switched off with it.
			if (Details.HullSettings.bAllowNonConvex)
			{
				Details.HullSettings.bCalculateOnSave = false;
			}
		});
}

bool FNWorldAssemblyEditorCellCommands::ToggleHullAllowNonConvex_IsActionChecked()
{
	return FNWorldAssemblyEditorCellOperations::QueryCellRootDetails(
		[](const FNCellRootDetails& Details) { return Details.HullSettings.bAllowNonConvex; });
}

void FNWorldAssemblyEditorCellCommands::ToggleVoxelCalculateOnSave()
{
	FNWorldAssemblyEditorCellOperations::ModifyCellRootDetails(
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCellCommands_ToggleVoxelCalculateOnSave", "Toggle Calculate Voxel On Save"),
		[](FNCellRootDetails& Details)
		{
			Details.VoxelSettings.bCalculateOnSave = !Details.VoxelSettings.bCalculateOnSave;
		});
}

bool FNWorldAssemblyEditorCellCommands::ToggleVoxelCalculateOnSave_IsActionChecked()
{
	return FNWorldAssemblyEditorCellOperations::QueryCellRootDetails(
		[](const FNCellRootDetails& Details) { return Details.VoxelSettings.bCalculateOnSave; });
}

void FNWorldAssemblyEditorCellCommands::ToggleVoxelData()
{
	FNWorldAssemblyEditorCellOperations::ModifyCellRootDetails(
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCellCommands_ToggleVoxelData", "Use Voxel Data"),
		[](FNCellRootDetails& Details)
		{
			Details.VoxelSettings.bUseVoxelData = !Details.VoxelSettings.bUseVoxelData;

			// Clear it here if someone uses the menu option
			if (!Details.VoxelSettings.bUseVoxelData)
			{
				Details.VoxelData = FNCellVoxelData();
			}
		});
}

bool FNWorldAssemblyEditorCellCommands::ToggleVoxelData_IsActionChecked()
{
	return FNWorldAssemblyEditorCellOperations::QueryCellRootDetails(
		[](const FNCellRootDetails& Details) { return Details.VoxelSettings.bUseVoxelData; });
}
