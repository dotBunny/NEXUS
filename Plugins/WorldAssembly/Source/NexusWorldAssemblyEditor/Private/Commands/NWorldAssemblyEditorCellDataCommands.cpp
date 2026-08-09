// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Commands/NWorldAssemblyEditorCellDataCommands.h"

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

FNWorldAssemblyEditorCellDataCommands& FNWorldAssemblyEditorCellDataCommands::Get()
{
	static FNWorldAssemblyEditorCellDataCommands Instance;
	return Instance;
}

void FNWorldAssemblyEditorCellDataCommands::Register(const TSharedRef<FBindingContext>& Context)
{
	FNWorldAssemblyEditorCellDataCommands& Commands = Get();

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_CaptureThumbnail,
		"NWorldAssembly.NCell.CaptureThumbnails",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CaptureThumbnails", "Capture Thumbnails"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CaptureThumbnails_Tooltip", "Captures the active viewport (minus widgets) as the thumbnails for the level containing the NCell, and the NCell data asset."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_ResetCell,
		"NWorldAssembly.NCell.ResetCell",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ResetCell", "Reset"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ResetCell_Tooltip", "Reset the cell data."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Reset"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_SaveCell,
		"NWorldAssembly.NCell.SaveCell",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_SaveCell", "Force Save"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_SaveCell_Tooltip", "Forcibly write out the Cells data to the sidecar file."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Save"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_AddActor,
		"NWorldAssembly.NCell.AddActor",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_AddActor", "Add Cell Actor"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_AddActor_Tooltip", "Create the singleton-like actor which will facilitate creating a NCell from the level it is placed in."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.AddNCellActor"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_RemoveActor,
		"NWorldAssembly.NCell.RemoveActor",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_RemoveActor", "Remove Actor"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_RemoveActor_Tooltip", "Removes the cell actor, no longer making this a cell."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.RemoveNCellActor"),
		EUserInterfaceActionType::Button, FInputChord());

	Commands.CommandList = MakeShared<FUICommandList>();

	Commands.CommandList->MapAction(Commands.CommandInfo_CaptureThumbnail,
		FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCellOperations::CaptureThumbnail),
		FCanExecuteAction::CreateStatic(&CaptureThumbnail_CanExecute));

	Commands.CommandList->MapAction(Commands.CommandInfo_ResetCell,
		FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCellOperations::ResetCell),
		FCanExecuteAction::CreateStatic(&CanEditCell));

	Commands.CommandList->MapAction(Commands.CommandInfo_SaveCell,
		FExecuteAction::CreateStatic(&SaveCell),
		FCanExecuteAction::CreateStatic(&CanEditCell));

	Commands.CommandList->MapAction(Commands.CommandInfo_AddActor,
	FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCellOperations::AddActor),
	FCanExecuteAction::CreateStatic(&AddActor_CanExecute));

	Commands.CommandList->MapAction(Commands.CommandInfo_RemoveActor,
		FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCellOperations::RemoveActor),
		FCanExecuteAction::CreateStatic(&CanEditCell));
}

bool FNWorldAssemblyEditorCellDataCommands::CanEditCell()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;
	return FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld();
}

bool FNWorldAssemblyEditorCellDataCommands::AddActor_CanExecute()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	// One cell per level, and never in a level that assembles them: a cell is a building block an operation places,
	// while a level holding organs is the world those blocks are placed into. Making a level both is not a thing the
	// assembly pipeline can act on, so the button greys out rather than letting it be authored.
	return !UNWorldAssemblyEdMode::HasCellActor() && !FNWorldAssemblyEditorUtils::IsOrganComponentPresentInCurrentWorld();
}

void FNWorldAssemblyEditorCellDataCommands::SaveCell()
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

void FNWorldAssemblyEditorCellDataCommands::TagIgnore()
{
	FNWorldAssemblyEditorTagOperations::ToggleTagOnSelection(
		NEXUS::WorldAssembly::ActorTags::CellIgnore,
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCellDataCommands_TagIgnore_Add", "Add CellIgnore Tags"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorCellDataCommands_TagIgnore_Remove", "Remove CellIgnore Tags"));
}

bool FNWorldAssemblyEditorCellDataCommands::TagIgnore_CanExecute()
{
	// Tagging writes to the actors and opens a transaction, so it is authoring work like the rest of the cell
	// commands — not something to run against a play world.
	if (FNEditorUtils::IsPlayInEditor()) return false;

	return UNWorldAssemblyEdMode::HasCellActor()
		&& FNEditorUtils::HasActorsSelected()
		&& !FNWorldAssemblyEditorUtils::IsCellActorSelected();
}

bool FNWorldAssemblyEditorCellDataCommands::CaptureThumbnail_CanExecute()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	const UWorld* World = FNEditorUtils::GetCurrentWorld();
	return World != nullptr && !FNEditorUtils::IsUnsavedWorld(World);
}