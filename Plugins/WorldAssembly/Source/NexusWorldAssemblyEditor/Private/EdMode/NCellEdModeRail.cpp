// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "EdMode/NCellEdModeRail.h"

#include "Editor.h"
#include "NEditorUtils.h"
#include "NUIEditorStyle.h"
#include "NWorldAssemblyEditorCommands.h"
#include "NWorldAssemblyEditorStyle.h"
#include "NWorldAssemblyEditorUserSettings.h"
#include "NWorldAssemblyEditorUtils.h"
#include "EdMode/NWorldAssemblyEdMode.h"
#include "EdMode/NWorldAssemblyEdModePaletteCommands.h"
#include "EdMode/NWorldAssemblyEdModeToolCommands.h"
#include "NWorldAssemblyMinimal.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellRootDetails.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/UICommandList.h"
#include "NWorldAssemblyEditorCellUtils.h"
#include "NWorldAssemblyEditorTagUtils.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

TSharedPtr<FUICommandList> FNCellEdModeRail::CategoryCommandList;

TSharedPtr<FUICommandInfo> FNCellEdModeRail::CommandInfo_SelectActor;
TSharedPtr<FUICommandInfo> FNCellEdModeRail::CommandInfo_TagIgnore;
TSharedPtr<FUICommandInfo> FNCellEdModeRail::CommandInfo_CalculateAll;
TSharedPtr<FUICommandInfo> FNCellEdModeRail::CommandInfo_CalculateBounds;
TSharedPtr<FUICommandInfo> FNCellEdModeRail::CommandInfo_CalculateHull;
TSharedPtr<FUICommandInfo> FNCellEdModeRail::CommandInfo_CalculateVoxelData;
TSharedPtr<FUICommandInfo> FNCellEdModeRail::CommandInfo_ToggleDrawVoxelData;
TSharedPtr<FUICommandInfo> FNCellEdModeRail::CommandInfo_ToggleDrawBounds;
TSharedPtr<FUICommandInfo> FNCellEdModeRail::CommandInfo_ToggleDrawHull;
TSharedPtr<FUICommandInfo> FNCellEdModeRail::CommandInfo_ToggleDrawFillBounds;
TSharedPtr<FUICommandInfo> FNCellEdModeRail::CommandInfo_ToggleBoundsCalculateOnSave;
TSharedPtr<FUICommandInfo> FNCellEdModeRail::CommandInfo_ToggleHullCalculateOnSave;
TSharedPtr<FUICommandInfo> FNCellEdModeRail::CommandInfo_ToggleHullAllowNonConvex;
TSharedPtr<FUICommandInfo> FNCellEdModeRail::CommandInfo_ToggleVoxelCalculateOnSave;
TSharedPtr<FUICommandInfo> FNCellEdModeRail::CommandInfo_ToggleVoxelData;

void FNCellEdModeRail::RegisterCommands(const TSharedRef<FBindingContext>& Context)
{
	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_CalculateAll,
		"NWorldAssembly.NCell.CalculateAll",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateAll", "Calculate All"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateAll_Tooltip", "Calculate all data related to the cell."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_CalculateBounds,
		"NWorldAssembly.NCell.CalculateBounds",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateBounds", "Calculate Bounds"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateBounds_Tooltip", "Calculate bounds for the cell."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Rail.Cell.CalculateCellBounds"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_CalculateHull,
		"NWorldAssembly.NCell.CalculateHull",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateHull", "Calculate Hull"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateHull_Tooltip", "Calculate convex hull for the cell."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Rail.Cell.CalculateCellHull"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_CalculateVoxelData,
		"NWorldAssembly.NCell.CalculateVoxelData",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateVoxelData", "Calculate Voxel"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CalculateVoxelData_Tooltip", "Calculate voxel data for the cell."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Rail.Cell.CalculateCellVoxel"),
		EUserInterfaceActionType::Button, FInputChord());



	// No icon on any of the Quick Options below, and MakeCommandInfo takes one positionally so it is stated rather
	// than omitted. CreateCheckList draws them as labelled checkboxes off GetLabel and GetDescription alone, and the
	// Keyboard Shortcuts editor lists a command by name and chord — so an icon here would draw nowhere.
	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_ToggleBoundsCalculateOnSave,
		"NWorldAssembly.NCell.ToggleBoundsCalculateOnSave",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleBoundsCalculateOnSave", "Calculate Bounds On Save"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleBoundsCalculateOnSave_Tooltip", "Calculates the bounds of the cell when the level is saved."),
		FSlateIcon(),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_ToggleHullAllowNonConvex,
		"NWorldAssembly.NCell.ToggleHullAllowNonConvex",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleHullAllowNonConvex", "Allow Non-Convex Hull"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleHullAllowNonConvex_Tooltip", "Allows for more complex collision mesh to be used instead of optimized convex hull."),
		FSlateIcon(),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_ToggleHullCalculateOnSave,
		"NWorldAssembly.NCell.ToggleHullCalculateOnSave",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleHullCalculateOnSave", "Calculate Hull On Save"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleHullCalculateOnSave_Tooltip", "Calculates the hull of the cell when the level is saved."),
		FSlateIcon(),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_ToggleVoxelCalculateOnSave,
		"NWorldAssembly.NCell.ToggleVoxelCalculateOnSave",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleVoxelCalculateOnSave", "Calculate Voxel Data On Save"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleVoxelCalculateOnSave_Tooltip", "Calculates the voxel data of the cell when the level is saved."),
		FSlateIcon(),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_ToggleVoxelData,
		"NWorldAssembly.NCell.ToggleVoxelData",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleVoxelData", "Use Voxel Data w/ Cell"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleVoxelData_Tooltip", "Should voxel data be generated and associated to this cell?"),
		FSlateIcon(),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_SelectActor,
		"NWorldAssembly.NCell.SelectActor",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_SelectActor", "Select Actor"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_SelectActor_Tooltip", "Select the NCellActor in the level."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Select"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_ToggleDrawVoxelData,
		"NWorldAssembly.NCell.ToggleDrawVoxelData",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_DrawVoxelData", "Draw Voxel Data"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_DrawVoxelData_Tooltip", "Toggle drawing calculated voxel data for the NCell."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.Voxel.Points"),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	// Iconless for the same reason the Quick Options above are — they draw as check-list rows. What they are not is
	// Quick Options: those write into the cell that ships, while these two are this user's own view state, and they
	// hold wherever a cell draws rather than only on the focused one.
	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_ToggleDrawBounds,
		"NWorldAssembly.NCell.ToggleDrawBounds",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleDrawBounds", "Draw Bounds"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleDrawBounds_Tooltip", "Draw the wire box of a cell's bounds in the viewport."),
		FSlateIcon(),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_ToggleDrawHull,
		"NWorldAssembly.NCell.ToggleDrawHull",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleDrawHull", "Draw Hull"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleDrawHull_Tooltip", "Draw a cell's convex hull, its collision mesh, in the viewport."),
		FSlateIcon(),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_ToggleDrawFillBounds,
		"NWorldAssembly.NCell.ToggleDrawFillBounds",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleDrawFillBounds", "Draw Fill Bounds"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ToggleDrawFillBounds_Tooltip", "Draw the box at each junction previewing the volume its filler would occupy."),
		FSlateIcon(),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	// Cell-scoped despite acting on the actor selection: the tag it toggles only means anything to a cell's
	// bounds/hull/voxel calculations, which is why it sits with the Cell actions rather than the world ones.
	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_TagIgnore,
		"NWorldAssembly.NCell.TagIgnore",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_TagIgnore", "Cell Collision"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_TagIgnore_Tooltip", "Toggles the necessary tag to have the selected actors ignored when calculating the bounds/hull/etc for a Cell."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Tag"),
		EUserInterfaceActionType::Button, FInputChord());

	using FOperations = FNWorldAssemblyEditorCellUtils;
	const FCanExecuteAction CanEditCell = FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorUtils::CanEditCell);

	CategoryCommandList = FNWorldAssemblyEditorCommands::MakeCommandList({
		{ CommandInfo_SelectActor,                 FExecuteAction::CreateStatic(&SelectActor),                                FCanExecuteAction::CreateStatic(&SelectActor_CanExecute) },
		{ CommandInfo_TagIgnore,                   FExecuteAction::CreateStatic(&TagIgnore),                                  FCanExecuteAction::CreateStatic(&TagIgnore_CanExecute) },
		{ CommandInfo_CalculateAll,                FExecuteAction::CreateStatic(&FOperations::CalculateAll),                  FCanExecuteAction::CreateStatic(&Calculate_CanExecute) },
		{ CommandInfo_CalculateBounds,             FExecuteAction::CreateStatic(&FOperations::CalculateBounds),               FCanExecuteAction::CreateStatic(&Calculate_CanExecute) },
		{ CommandInfo_CalculateHull,               FExecuteAction::CreateStatic(&FOperations::CalculateHull),                 FCanExecuteAction::CreateStatic(&Calculate_CanExecute) },
		{ CommandInfo_CalculateVoxelData,          FExecuteAction::CreateStatic(&FOperations::CalculateVoxelData),            FCanExecuteAction::CreateStatic(&CalculateVoxelData_CanExecute) },
		{ CommandInfo_ToggleDrawVoxelData,         FExecuteAction::CreateStatic(&ToggleDrawVoxelData),                        CanEditCell, FIsActionChecked::CreateStatic(&ToggleDrawVoxelData_IsActionChecked) },
		// No CanExecute, unlike every row around them: those act on the focused cell and have nothing to do without
		// one, while what to draw stays a decision worth making whenever the category is on screen.
		{ CommandInfo_ToggleDrawBounds,            FExecuteAction::CreateStatic(&ToggleDrawBounds),                           FCanExecuteAction(), FIsActionChecked::CreateStatic(&ToggleDrawBounds_IsActionChecked) },
		{ CommandInfo_ToggleDrawHull,              FExecuteAction::CreateStatic(&ToggleDrawHull),                             FCanExecuteAction(), FIsActionChecked::CreateStatic(&ToggleDrawHull_IsActionChecked) },
		{ CommandInfo_ToggleDrawFillBounds,        FExecuteAction::CreateStatic(&ToggleDrawFillBounds),                       FCanExecuteAction(), FIsActionChecked::CreateStatic(&ToggleDrawFillBounds_IsActionChecked) },
		{ CommandInfo_ToggleBoundsCalculateOnSave, FExecuteAction::CreateStatic(&FOperations::ToggleBoundsCalculateOnSave),   CanEditCell, FIsActionChecked::CreateStatic(&FOperations::ToggleBoundsCalculateOnSave_IsActionChecked) },
		{ CommandInfo_ToggleHullCalculateOnSave,   FExecuteAction::CreateStatic(&FOperations::ToggleHullCalculateOnSave),     CanEditCell, FIsActionChecked::CreateStatic(&FOperations::ToggleHullCalculateOnSave_IsActionChecked) },
		{ CommandInfo_ToggleHullAllowNonConvex,    FExecuteAction::CreateStatic(&FOperations::ToggleHullAllowNonConvex),      CanEditCell, FIsActionChecked::CreateStatic(&FOperations::ToggleHullAllowNonConvex_IsActionChecked) },
		{ CommandInfo_ToggleVoxelCalculateOnSave,  FExecuteAction::CreateStatic(&FOperations::ToggleVoxelCalculateOnSave),    CanEditCell, FIsActionChecked::CreateStatic(&FOperations::ToggleVoxelCalculateOnSave_IsActionChecked) },
		{ CommandInfo_ToggleVoxelData,             FExecuteAction::CreateStatic(&FOperations::ToggleVoxelData),               CanEditCell, FIsActionChecked::CreateStatic(&FOperations::ToggleVoxelData_IsActionChecked) },
	});
}

TSharedRef<FUICommandList> FNCellEdModeRail::GetCommandList()
{
	return CategoryCommandList.ToSharedRef();
}

void FNCellEdModeRail::SelectActor()
{
	GEditor->SelectNone(false, true);
	GEditor->SelectActor(FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld(),
		true, true, true, true);
}

bool FNCellEdModeRail::SelectActor_CanExecute()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;
	return UNWorldAssemblyEdMode::HasCellActor() && GEditor->CanSelectActor(UNWorldAssemblyEdMode::GetCellActor(), false);
}

bool FNCellEdModeRail::Calculate_CanExecute()
{
	return FNWorldAssemblyEditorUtils::CanEditCell() && UNWorldAssemblyEdMode::IsTerrainSettled();
}

bool FNCellEdModeRail::CalculateVoxelData_CanExecute()
{
	if (!Calculate_CanExecute()) return false;

	const ANCellActor* CellActor = FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld();
	if (CellActor == nullptr) return false;
	return CellActor->GetCellRoot()->Details.VoxelSettings.bUseVoxelData;
}

void FNCellEdModeRail::TagIgnore()
{
	FNWorldAssemblyEditorTagUtils::ToggleTagOnSelection(
		NEXUS::WorldAssembly::ActorTags::CellIgnore,
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEdModeCellRail_TagIgnore_Add", "Add CellIgnore Tags"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEdModeCellRail_TagIgnore_Remove", "Remove CellIgnore Tags"));
}

FSlateIcon FNCellEdModeRail::TagIgnoreIcon()
{
	// Asks the same question ToggleTagOnSelection asks to pick its transaction, so the button cannot promise one thing
	// and do the other: any tagged actor in the selection means the next click strips the tag from all of them.
	const bool bWouldRemove = FNWorldAssemblyEditorTagUtils::IsTagOnAnySelectedActor(
		NEXUS::WorldAssembly::ActorTags::CellIgnore);

	return FSlateIcon(FNUIEditorStyle::GetStyleSetName(), bWouldRemove ? "Command.ToggleOn" : "Command.ToggleOff");
}

bool FNCellEdModeRail::TagIgnore_CanExecute()
{
	// Tagging writes to the actors and opens a transaction, so it is authoring work like the rest of the cell
	// commands — not something to run against a play world.
	if (FNEditorUtils::IsPlayInEditor()) return false;

	return UNWorldAssemblyEdMode::HasCellActor()
		&& FNEditorUtils::HasActorsSelected()
		&& !FNWorldAssemblyEditorUtils::IsCellActorSelected();
}

void FNCellEdModeRail::ToggleDrawVoxelData()
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

bool FNCellEdModeRail::ToggleDrawVoxelData_IsActionChecked()
{
	return UNWorldAssemblyEdMode::GetCellVoxelMode() != UNWorldAssemblyEdMode::ENCellVoxelMode::None;
}

void FNCellEdModeRail::ToggleDrawBounds()
{
	// Written through on every click rather than on mode exit: this is the user's preference for every level they
	// open next, and an editor that goes down without a clean shutdown should not take it with it.
	UNWorldAssemblyEditorUserSettings* Settings = UNWorldAssemblyEditorUserSettings::GetMutable();
	Settings->bDebugCellDrawBounds = !Settings->bDebugCellDrawBounds;
	Settings->SaveConfig();
}

bool FNCellEdModeRail::ToggleDrawBounds_IsActionChecked()
{
	return UNWorldAssemblyEditorUserSettings::Get()->bDebugCellDrawBounds;
}

void FNCellEdModeRail::ToggleDrawHull()
{
	UNWorldAssemblyEditorUserSettings* Settings = UNWorldAssemblyEditorUserSettings::GetMutable();
	Settings->bDebugCellDrawHull = !Settings->bDebugCellDrawHull;
	Settings->SaveConfig();
}

bool FNCellEdModeRail::ToggleDrawHull_IsActionChecked()
{
	return UNWorldAssemblyEditorUserSettings::Get()->bDebugCellDrawHull;
}

void FNCellEdModeRail::ToggleDrawFillBounds()
{
	UNWorldAssemblyEditorUserSettings* Settings = UNWorldAssemblyEditorUserSettings::GetMutable();
	Settings->bDebugCellDrawFillBounds = !Settings->bDebugCellDrawFillBounds;
	Settings->SaveConfig();
}

bool FNCellEdModeRail::ToggleDrawFillBounds_IsActionChecked()
{
	return UNWorldAssemblyEditorUserSettings::Get()->bDebugCellDrawFillBounds;
}

TSharedPtr<FUICommandInfo> FNCellEdModeRail::GetCategoryCommand() const
{
	return FNWorldAssemblyEdModePaletteCommands::Get().LoadCellPalette;
}

TAttribute<bool> FNCellEdModeRail::GetAvailable() const
{
	// Level contents rather than the focused actor. The category is worth showing whenever the level has a cell to
	// author, whether or not one is selected this instant; the buttons inside it are what go dead when nothing is
	// focused.
	return TAttribute<bool>::CreateStatic(&FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld);
}

bool FNCellEdModeRail::ShouldAutoSelect() const
{
	// A level holding a cell actor is a cell, and authoring it is the whole of what the mode is for there — so opening
	// on World, which offers only the Add Cell Actor that has already been run, wastes the user's first click. No test
	// of its own: available already means the level has a cell, which is the same question.
	//
	// Cell rather than Cell Data or Junction, which share that availability: those two are views onto a cell that
	// already exists, while this is where it is built.
	return true;
}

TSharedPtr<SWidget> FNCellEdModeRail::CreateContent() const
{
	const FNWorldAssemblyEdModeToolCommands& ToolCommands = FNWorldAssemblyEdModeToolCommands::Get();

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			// Unheaded: the tiles are the first thing in the Cell category and carry their own labels, so a "Tools"
			// heading over them only repeats what they already say.
			CreateCommandPalette(
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
			CreateCommandList(
				{
					CommandInfo_SelectActor,
					CommandInfo_CalculateBounds,
					CommandInfo_CalculateHull,
					CommandInfo_CalculateAll,
					//CommandInfo_CalculateVoxelData,
				})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateGroupSeparator(LOCTEXT("CellSelectedActorSeparator", "SELECTED ACTOR"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			// Icon bound rather than taken from the command: it reports whether the tag is already on the selection,
			// which the registered icon cannot do.
			CreateCommandList(
				{
					{ CommandInfo_TagIgnore, TAttribute<FSlateIcon>::CreateStatic(&TagIgnoreIcon) }
				})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateGroupSeparator(LOCTEXT("CellQuickOptionsSeparator", "QUICK OPTIONS"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateCheckList(
				{
					CommandInfo_ToggleBoundsCalculateOnSave,
					CommandInfo_ToggleHullCalculateOnSave,
					CommandInfo_ToggleHullAllowNonConvex,
					//CommandInfo_ToggleVoxelCalculateOnSave,
					//CommandInfo_ToggleVoxelData,
				})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateGroupSeparator(LOCTEXT("CellDisplaySeparator", "DISPLAY"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			// Ruled off from the Quick Options rather than folded in with them, because the two persist somewhere
			// different: those are authored into the cell and travel with it, these are this machine's own view
			// state. One check list holding both would say they are the same kind of setting.
			CreateCheckList(
				{
					CommandInfo_ToggleDrawBounds,
					CommandInfo_ToggleDrawHull,
					CommandInfo_ToggleDrawFillBounds,
				})
		];
}

#undef LOCTEXT_NAMESPACE
