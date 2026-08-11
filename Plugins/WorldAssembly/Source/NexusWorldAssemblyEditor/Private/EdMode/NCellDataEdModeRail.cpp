// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "EdMode/NCellDataEdModeRail.h"

#include "NEditorUtils.h"
#include "NTerrainUtils.h"
#include "NUIEditorStyle.h"
#include "NWorldAssemblyEditorCommands.h"
#include "NWorldAssemblyEditorStyle.h"
#include "NWorldAssemblyEditorUtils.h"
#include "EdMode/NWorldAssemblyEdModePaletteCommands.h"
#include "NWorldAssemblyUtils.h"
#include "Cell/NCellActor.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/UICommandList.h"
#include "NWorldAssemblyEditorCellUtils.h"
#include "Styling/AppStyle.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

TSharedPtr<FUICommandList> FNCellDataEdModeRail::CategoryCommandList;

TSharedPtr<FUICommandInfo> FNCellDataEdModeRail::CommandInfo_RemoveActor;
TSharedPtr<FUICommandInfo> FNCellDataEdModeRail::CommandInfo_CaptureThumbnail;
TSharedPtr<FUICommandInfo> FNCellDataEdModeRail::CommandInfo_ResetCell;
TSharedPtr<FUICommandInfo> FNCellDataEdModeRail::CommandInfo_SaveCell;

void FNCellDataEdModeRail::RegisterCommands(const TSharedRef<FBindingContext>& Context)
{
	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_CaptureThumbnail,
		"NWorldAssembly.NCell.CaptureThumbnails",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CaptureThumbnails", "Capture Thumbnails"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_CaptureThumbnails_Tooltip", "Captures the active viewport (minus widgets) as the thumbnails for the level containing the NCell, and the NCell data asset."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_ResetCell,
		"NWorldAssembly.NCell.ResetCell",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ResetCell", "Reset"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_ResetCell_Tooltip", "Reset the cell data."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Reset"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_SaveCell,
		"NWorldAssembly.NCell.SaveCell",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_SaveCell", "Force Save"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_SaveCell_Tooltip", "Forcibly write out the Cells data to the sidecar file."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Save"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_RemoveActor,
		"NWorldAssembly.NCell.RemoveActor",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_RemoveActor", "Remove Actor"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_RemoveActor_Tooltip", "Removes the cell actor, no longer making this a cell."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Rail.Cell.Icon"),
		EUserInterfaceActionType::Button, FInputChord());

	using FOperations = FNWorldAssemblyEditorCellUtils;
	using FUtils = FNWorldAssemblyEditorUtils;

	CategoryCommandList = FNWorldAssemblyEditorCommands::MakeCommandList({
		{ CommandInfo_CaptureThumbnail, FExecuteAction::CreateStatic(&FOperations::CaptureThumbnail), FCanExecuteAction::CreateStatic(&CaptureThumbnail_CanExecute) },
		{ CommandInfo_ResetCell,        FExecuteAction::CreateStatic(&FOperations::ResetCell),        FCanExecuteAction::CreateStatic(&FUtils::CanEditCell) },
		{ CommandInfo_SaveCell,         FExecuteAction::CreateStatic(&SaveCell),                      FCanExecuteAction::CreateStatic(&FUtils::CanEditCell) },
		{ CommandInfo_RemoveActor,      FExecuteAction::CreateStatic(&FOperations::RemoveActor),      FCanExecuteAction::CreateStatic(&FUtils::CanEditCell) },
	});
}

TSharedRef<FUICommandList> FNCellDataEdModeRail::GetCommandList()
{
	return CategoryCommandList.ToSharedRef();
}

void FNCellDataEdModeRail::SaveCell()
{
	UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	ANCellActor* CellActor = FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);

	// Here rather than inside SaveCell: this is the last point before the engine's save machinery takes over, and the
	// wait must not run once it has — see FNTerrainUtils::WaitForSettle's remark.
	if (CellActor != nullptr)
	{
		FNTerrainUtils::WaitForSettle(CellActor->GetLevel());
	}

	FNWorldAssemblyEditorUtils::SaveCell(CurrentWorld, CellActor, true);
}

bool FNCellDataEdModeRail::CaptureThumbnail_CanExecute()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	const UWorld* World = FNEditorUtils::GetCurrentWorld();
	return World != nullptr && !FNEditorUtils::IsUnsavedWorld(World);
}

TSharedPtr<FUICommandInfo> FNCellDataEdModeRail::GetCategoryCommand() const
{
	return FNWorldAssemblyEdModePaletteCommands::Get().LoadCellDataPalette;
}

TAttribute<bool> FNCellDataEdModeRail::GetAvailable() const
{
	// Level contents rather than the focused actor. The category is worth showing whenever the level has a cell to
	// author, whether or not one is selected this instant; the buttons inside it are what go dead when nothing is
	// focused.
	return TAttribute<bool>::CreateStatic(&FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld);
}

TSharedPtr<SWidget> FNCellDataEdModeRail::CreateContent() const
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateCommandList(
				{
					CommandInfo_RemoveActor,
					CommandInfo_CaptureThumbnail,
					CommandInfo_SaveCell,
					CommandInfo_ResetCell
				})
		];
}

#undef LOCTEXT_NAMESPACE
