// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEditorCommands.h"

#include "AssetDefinitions/AssetDefinition_NCell.h"
#include "EditorAssetLibrary.h"
#include "FileHelpers.h"
#include "NCoreEditorMinimal.h"
#include "Cell/NCellActor.h"
#include "Cell/NCellJunctionComponent.h"
#include "NEditorUtils.h"
#include "NProcGenEditorUtils.h"
#include "NProcGenEdMode.h"
#include "NProcGenUtils.h"
#include "NUIEditorStyle.h"
#include "Selection.h"

#define LOCTEXT_NAMESPACE "NexusProcGenEditor"

void FNProcGenEditorCommands::RegisterCommands()
{
	// Build NCell Command Info
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_NCellCalculateAll,
		"NProcGen.NCell.CalculateAll",
		LOCTEXT("Command_NCell_CalculateAll", "Calculate All"),
		LOCTEXT("Command_NCell_CalculateAll_Tooltip", "Calculate all data related to the cell."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"),
		EUserInterfaceActionType::Button, FInputGesture());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_NCellCalculateBounds,
		"NProcGen.NCell.CalculateBounds",
		LOCTEXT("Command_NCell_CalculateBounds", "Calculate Bounds"),
		LOCTEXT("Command_NCell_CalculateBounds_Tooltip", "Calculate bounds for the cell."),
		FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.CalculateBounds"),
		EUserInterfaceActionType::Button, FInputGesture());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_NCellCalculateHull,
		"NProcGen.NCell.CalculateHull",
		LOCTEXT("Command_NCell_CalculateHull", "Calculate Hull"),
		LOCTEXT("Command_NCell_CalculateHull_Tooltip", "Calculate convex hull for the cell."),
		FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.CalculateHull"),
		EUserInterfaceActionType::Button, FInputGesture());
	
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_NCellResetCell,
		"NProcGen.NCell.ResetCell",
		LOCTEXT("Command_NCell_ResetCell", "Reset Cell"),
		LOCTEXT("Command_NCell_ResetCell_Tooltip", "Reset the cell data."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Reset"),
		EUserInterfaceActionType::Button, FInputGesture());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_NCellRemoveActor,
		"NProcGen.NCell.RemoveActor",
		LOCTEXT("Command_NCell_RemoveActor", "Remove Actor"),
		LOCTEXT("Command_NCell_RemoveActor_Tooltip", "Removes the cell actor, no longer making this a cell."),
		FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.RemoveNCellActor"),
		EUserInterfaceActionType::Button, FInputGesture());

	// Create NCell Command List
	CommandList_NCell = MakeShareable(new FUICommandList);

	// Map NCell Actions

	CommandList_NCell->MapAction(Get().CommandInfo_NCellCalculateAll,
		FExecuteAction::CreateStatic(&OnNCellCalculateAll),
		FCanExecuteAction::CreateStatic(&OnNCellCalculateAll_CanExecute));

	CommandList_NCell->MapAction(Get().CommandInfo_NCellCalculateBounds,
		FExecuteAction::CreateStatic(&OnNCellCalculateBounds),
		FCanExecuteAction::CreateStatic(&OnNCellCalculateBounds_CanExecute));

	CommandList_NCell->MapAction(Get().CommandInfo_NCellCalculateHull,
	FExecuteAction::CreateStatic(&OnNCellCalculateHull),
	FCanExecuteAction::CreateStatic(&OnNCellCalculateHull_CanExecute));
	
	CommandList_NCell->MapAction(Get().CommandInfo_NCellResetCell,
	FExecuteAction::CreateStatic(&OnNCellResetCell),
	FCanExecuteAction::CreateStatic(&OnNCellResetCell_CanExecute));

	CommandList_NCell->MapAction(Get().CommandInfo_NCellRemoveActor,
FExecuteAction::CreateStatic(&OnNCellRemoveActor),
	FCanExecuteAction::CreateStatic(&OnNCellRemoveActor_CanExecute));

	// Build NCellJunction Command Info
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_NCellJunctionAddComponent,
		"NProcGen.NCellJunction.AddComponent",
		LOCTEXT("Command_NCellJunction_AddComponent", "Add Component"),
		LOCTEXT("Command_NCellJunction_AddComponent_Tooltip", "Add a NCellJunction component to current actor."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"), // TODO: Create icon
		EUserInterfaceActionType::Button, FInputGesture());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_NCellJunctionSelectComponent,
	"NProcGen.NCellJunction.SelectComponent",
	LOCTEXT("Command_NCellJunction_SelectComponent", "Select Component"),
	LOCTEXT("Command_NCellJunction_SelectComponent_Tooltip", "Select a NCellJunction in the level."),
	FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"), // TODO: Create icon
	EUserInterfaceActionType::Button, FInputGesture());

	// Create NCell Command List
	CommandList_NCellJunction = MakeShareable(new FUICommandList);

	CommandList_NCellJunction->MapAction(Get().CommandInfo_NCellJunctionAddComponent,
FExecuteAction::CreateStatic(&OnNCellJunctionAddComponent),
	FCanExecuteAction::CreateStatic(&OnNCellJunctionAddComponent_CanExecute));
}

void FNProcGenEditorCommands::NProcGenEdMode()
{
	GLevelEditorModeTools().ActivateMode(FNProcGenEdMode::Identifier);
}

bool FNProcGenEditorCommands::NProcGenEdMode_CanExecute()
{
	return !FNProcGenEdMode::IsActive();
}

bool FNProcGenEditorCommands::NProcGenEdMode_CanShow()
{
	if (FNProcGenEdMode::IsActive()) return false;
	return FNProcGenEditorUtils::IsNCellActorPresentInCurrentWorld();
}

void FNProcGenEditorCommands::NCellActorEditHullMode()
{
	FNProcGenEdMode::SetCellEdMode(FNProcGenEdMode::ENCellEdMode::NCell_Hull);
}

FSlateIcon FNProcGenEditorCommands::NCellActorEditHullMode_GetIcon()
{
	if (FNProcGenEdMode::GetNCellEdMode() == FNProcGenEdMode::ENCellEdMode::NCell_Hull)
	{
		return FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.Hull.Selected");
	}
	return FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.Hull");
}

void FNProcGenEditorCommands::NCellActorEditBoundsMode()
{
	FNProcGenEdMode::SetCellEdMode(FNProcGenEdMode::ENCellEdMode::NCell_Bounds);
}

FSlateIcon FNProcGenEditorCommands::NCellActorEditBoundsMode_GetIcon()
{
	if (FNProcGenEdMode::GetNCellEdMode() == FNProcGenEdMode::ENCellEdMode::NCell_Bounds)
	{
		return FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.Bounds.Selected");
	}
	return FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.Bounds");
}

void FNProcGenEditorCommands::NCellAddActor()
{
	if (UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld())
	{
		if (FNEditorUtils::IsUnsavedWorld(CurrentWorld))
		{
			const EAppReturnType::Type Choice = FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Type::YesNo,
				FText::FromString(TEXT("You need to save the world/map that you are working in before creating a NCellActor.\n\nDo you wish to save the map now?")),
				FText::FromString(TEXT("NEXUS: ProcGen")));
			switch (Choice)
			{
			case EAppReturnType::No:
				NE_LOG(Error, TEXT("Unable to add NCellActor to an unsaved world."))
				return;
			case EAppReturnType::Yes:
				if (!FEditorFileUtils::SaveLevel(CurrentWorld->GetCurrentLevel()))
				{
					NE_LOG(Error, TEXT("Unable to add NCellActor to an unsaved world."))
					return;
				}
				break;
			default:
				NE_LOG(Error, TEXT("Unable to add NCellActor to an unsaved world."))
				return;
			}
		}
		
		ANCellActor* SpawnedActor = CurrentWorld->SpawnActor<ANCellActor>(ANCellActor::StaticClass(), FTransform::Identity, FActorSpawnParameters());
		FNProcGenEditorUtils::SaveNCell(CurrentWorld, SpawnedActor);
	}
}

bool FNProcGenEditorCommands::NCellAddActor_CanExecute()
{
	return !FNProcGenEdMode::HasNCellActor();
}

bool FNProcGenEditorCommands::NCellAddActor_CanShow()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;
	return FNProcGenEdMode::IsActive() && !FNProcGenEdMode::HasNCellActor();
}

void FNProcGenEditorCommands::NCellSelectActor()
{
	GEditor->SelectActor(FNProcGenEditorUtils::GetNCellActorFromCurrentWorld(),
		true, true, true, true);
}

bool FNProcGenEditorCommands::NCellSelectActor_CanExecute()
{
	return FNProcGenEdMode::HasNCellActor() && GEditor->CanSelectActor(FNProcGenEdMode::GetNCellActor(), false);
}

bool FNProcGenEditorCommands::NCellSelectActor_CanShow()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;
	
	// Use cached version
	return FNProcGenEdMode::IsActive() && FNProcGenEdMode::HasNCellActor();
}

void FNProcGenEditorCommands::OnNCellCalculateAll()
{
	OnNCellCalculateBounds();
	OnNCellCalculateHull();
}

bool FNProcGenEditorCommands::OnNCellCalculateAll_CanExecute()
{
	return FNProcGenEditorUtils::IsNCellActorPresentInCurrentWorld();
}

void FNProcGenEditorCommands::OnNCellCalculateBounds()
{
	ANCellActor* CellActor = FNProcGenEditorUtils::GetNCellActorFromCurrentWorld();
	CellActor->CalculateBounds();
}

bool FNProcGenEditorCommands::OnNCellCalculateBounds_CanExecute()
{
	return FNProcGenEditorUtils::IsNCellActorPresentInCurrentWorld();
}

void FNProcGenEditorCommands::OnNCellCalculateHull()
{
	ANCellActor* CellActor = FNProcGenEditorUtils::GetNCellActorFromCurrentWorld();
	CellActor->CalculateHull();
}

bool FNProcGenEditorCommands::OnNCellCalculateHull_CanExecute()
{
	return FNProcGenEditorUtils::IsNCellActorPresentInCurrentWorld();
}

void FNProcGenEditorCommands::OnNCellResetCell()
{
	// Get the cell actor
	ANCellActor* CellActor = FNProcGenEditorUtils::GetNCellActorFromCurrentWorld();
	CellActor->CellRoot->Reset();

	// Get the cell
	UNCell* Cell = UAssetDefinition_NCell::GetOrCreatePackage( FNEditorUtils::GetCurrentWorld());
	Cell->Root = FNCellRootDetails();
	Cell->Junctions.Empty();

	// Update the data
	FNProcGenEditorUtils::UpdateNCell(Cell, CellActor);

	// Flag the actor as dirty
	CellActor->SetActorDirty();
}

bool FNProcGenEditorCommands::OnNCellResetCell_CanExecute()
{
	return FNProcGenEditorUtils::IsNCellActorPresentInCurrentWorld();
}

void FNProcGenEditorCommands::OnNCellRemoveActor()
{
	UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	ANCellActor* CellActor = FNProcGenUtils::GetNCellActorFromWorld(CurrentWorld, true);
	CellActor->Destroy();
	CurrentWorld->MarkPackageDirty();
	
	// Need to destroy the sidecar.
	if (const FString SidecarPath = UAssetDefinition_NCell::GetCellPackagePath(CurrentWorld->GetOutermost()->GetName());
		FPackageName::DoesPackageExist(SidecarPath))
		{
			UEditorAssetLibrary::DeleteAsset(SidecarPath);
		}
}

bool FNProcGenEditorCommands::OnNCellRemoveActor_CanExecute()
{
	return FNProcGenEditorUtils::IsNCellActorPresentInCurrentWorld();
}

void FNProcGenEditorCommands::OnNCellJunctionAddComponent()
{
	TArray<UNCellJunctionComponent*> OutComponents;
	TArray<UNCellJunctionComponent*> SelectComponents;
	bool bNeedsRefresh = false;
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		AActor* Actor = Cast<AActor>( *SelectedActor );
		if (!Actor) continue;
		
		Actor->GetComponents(UNCellJunctionComponent::StaticClass(), OutComponents, true);
		if (OutComponents.Num() == 0)
		{
			UNCellJunctionComponent* NewComponent = static_cast<UNCellJunctionComponent*>(Actor->AddComponentByClass(
				UNCellJunctionComponent::StaticClass(), true, FTransform::Identity, false));
			
			
			NewComponent->AttachToComponent(Actor->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
			Actor->AddInstanceComponent(NewComponent);
			NewComponent->RegisterComponent();

			SelectComponents.Add(NewComponent);
			bNeedsRefresh = true;
		}
	}

	// Refresh the details panel if needed
	if (bNeedsRefresh)
	{
		GEditor->SelectNone(false, true);
		for (UNCellJunctionComponent* Component : SelectComponents)
		{
			GEditor->SelectComponent(Component, true, true, true);
		}
	}
}

bool FNProcGenEditorCommands::OnNCellJunctionAddComponent_CanExecute()
{
	if (GEditor->GetSelectedActorCount() == 0) return false;
	
	TArray<UNCellJunctionComponent*> OutComponents;
	bool bFoundSpot = false;
	
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		AActor* Actor = Cast<AActor>( *SelectedActor );
		if (!Actor) continue;

		Actor->GetComponents(UNCellJunctionComponent::StaticClass(), OutComponents, true);

		if (OutComponents.Num() == 0)
		{
			bFoundSpot = true;
		}
	}
	return bFoundSpot;
}

void FNProcGenEditorCommands::OnNCellJunctionSelectComponent(UNCellJunctionComponent* Junction)
{
	GEditor->SelectNone(false, true);
	GEditor->SelectComponent(Junction, true, true, true);
}

bool FNProcGenEditorCommands::OnNCellJunctionSelectComponent_CanExecute(UNCellJunctionComponent* Junction)
{
	return true;
}

#undef LOCTEXT_NAMESPACE
