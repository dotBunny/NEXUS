// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEditorCommands.h"

#include "AssetViewUtils.h"
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
#include "AssetRegistry/AssetRegistryModule.h"
#include "Organ/NOrganGenerator.h"

#define LOCTEXT_NAMESPACE "NexusProcGenEditor"

void FNProcGenEditorCommands::RegisterCommands()
{
	// Build NCell Command Info
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellCaptureThumbnail,
			"NProcGen.NCell.CaptureThumbnail",
			LOCTEXT("Command_NCell_CaptureThumbnail", "Capture Thumbnail"),
			LOCTEXT("Command_NCell_CaptureThumbnail_Tooltip", "Captures the active viewport (minus widgets) as the thumbnail for the level containing the NCell."),
			FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"),
			EUserInterfaceActionType::Button, FInputChord());
	
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellCalculateAll,
		"NProcGen.NCell.CalculateAll",
		LOCTEXT("Command_NCell_CalculateAll", "Calculate All"),
		LOCTEXT("Command_NCell_CalculateAll_Tooltip", "Calculate all data related to the cell."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellCalculateBounds,
		"NProcGen.NCell.CalculateBounds",
		LOCTEXT("Command_NCell_CalculateBounds", "Calculate Bounds"),
		LOCTEXT("Command_NCell_CalculateBounds_Tooltip", "Calculate bounds for the cell."),
		FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.CalculateBounds"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellCalculateHull,
		"NProcGen.NCell.CalculateHull",
		LOCTEXT("Command_NCell_CalculateHull", "Calculate Hull"),
		LOCTEXT("Command_NCell_CalculateHull_Tooltip", "Calculate convex hull for the cell."),
		FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.CalculateHull"),
		EUserInterfaceActionType::Button, FInputChord());
	
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellCalculateVoxelData,
		"NProcGen.NCell.CalculateVoxelData",
		LOCTEXT("Command_NCell_CalculateVoxelData", "Calculate Voxel Data"),
		LOCTEXT("Command_NCell_CalculateVoxelData_Tooltip", "Calculate voxel data for the cell."),
		FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.CalculateVoxelData"),
		EUserInterfaceActionType::Button, FInputChord());
	
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellResetCell,
		"NProcGen.NCell.ResetCell",
		LOCTEXT("Command_NCell_ResetCell", "Reset Cell"),
		LOCTEXT("Command_NCell_ResetCell_Tooltip", "Reset the cell data."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Reset"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellRemoveActor,
		"NProcGen.NCell.RemoveActor",
		LOCTEXT("Command_NCell_RemoveActor", "Remove Actor"),
		LOCTEXT("Command_NCell_RemoveActor_Tooltip", "Removes the cell actor, no longer making this a cell."),
		FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.RemoveNCellActor"),
		EUserInterfaceActionType::Button, FInputChord());

	// Create NCell Command List
	CommandList_Cell = MakeShareable(new FUICommandList);

	// Map NCell Actions

	CommandList_Cell->MapAction(Get().CommandInfo_CellCaptureThumbnail,
		FExecuteAction::CreateStatic(&CellCaptureThumbnail),
		FCanExecuteAction::CreateStatic(&CellCaptureThumbnail_CanExecute));
	
	CommandList_Cell->MapAction(Get().CommandInfo_CellCalculateAll,
		FExecuteAction::CreateStatic(&CellCalculateAll),
		FCanExecuteAction::CreateStatic(&CellCalculateAll_CanExecute));

	CommandList_Cell->MapAction(Get().CommandInfo_CellCalculateBounds,
		FExecuteAction::CreateStatic(&CellCalculateBounds),
		FCanExecuteAction::CreateStatic(&CellCalculateBounds_CanExecute));

	CommandList_Cell->MapAction(Get().CommandInfo_CellCalculateHull,
	FExecuteAction::CreateStatic(&CellCalculateHull),
	FCanExecuteAction::CreateStatic(&CellCalculateHull_CanExecute));
	
	CommandList_Cell->MapAction(Get().CommandInfo_CellCalculateVoxelData,
FExecuteAction::CreateStatic(&CellCalculateVoxelData),
	FCanExecuteAction::CreateStatic(&CellCalculateVoxelData_CanExecute));
	
	CommandList_Cell->MapAction(Get().CommandInfo_CellResetCell,
	FExecuteAction::CreateStatic(&CellResetCell),
	FCanExecuteAction::CreateStatic(&CellResetCell_CanExecute));

	CommandList_Cell->MapAction(Get().CommandInfo_CellRemoveActor,
FExecuteAction::CreateStatic(&CellRemoveActor),
	FCanExecuteAction::CreateStatic(&CellRemoveActor_CanExecute));

	// Build NCellJunction Command Info
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellJunctionAddComponent,
		"NProcGen.NCellJunction.AddComponent",
		LOCTEXT("Command_NCellJunction_AddComponent", "Add Component"),
		LOCTEXT("Command_NCellJunction_AddComponent_Tooltip", "Add a NCellJunction component to current actor."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_CellJunctionSelectComponent,
	"NProcGen.NCellJunction.SelectComponent",
	LOCTEXT("Command_NCellJunction_SelectComponent", "Select Component"),
	LOCTEXT("Command_NCellJunction_SelectComponent_Tooltip", "Select a NCellJunction in the level."),
	FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"),
	EUserInterfaceActionType::Button, FInputChord());

	// Create NCell Command List
	CommandList_CellJunction = MakeShareable(new FUICommandList);

	CommandList_CellJunction->MapAction(Get().CommandInfo_CellJunctionAddComponent,
FExecuteAction::CreateStatic(&CellJunctionAddComponent),
	FCanExecuteAction::CreateStatic(&CellJunctionAddComponent_CanExecute));
	
	
	// Organ
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_OrganGenerate,
	"NProcGen.NOrganComponent.Generate",
	LOCTEXT("Command_NOrganComponent_Generate", "Generate"),
	LOCTEXT("Command_NOrganComponent_Generate_Tooltip", "Generate thingy"),
	FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"),
	EUserInterfaceActionType::Button, FInputChord());
	
	CommandList_Organ = MakeShareable(new FUICommandList);
	CommandList_Organ->MapAction(Get().CommandInfo_OrganGenerate,
		FExecuteAction::CreateStatic(&OrganGenerate),
		FCanExecuteAction::CreateStatic(&OrganGenerate_CanExecute));
}

void FNProcGenEditorCommands::ProcGenEdMode()
{
	GLevelEditorModeTools().ActivateMode(FNProcGenEdMode::Identifier);
}

bool FNProcGenEditorCommands::ProcGenEdMode_CanExecute()
{
	return !FNProcGenEdMode::IsActive();
}

bool FNProcGenEditorCommands::ProcGenEdMode_CanShow()
{
	if (FNProcGenEdMode::IsActive()) return false;
	return FNProcGenEditorUtils::IsOrganComponentPresentInCurrentWorld() || FNProcGenEditorUtils::IsCellActorPresentInCurrentWorld();
}

void FNProcGenEditorCommands::CellActorEditHullMode()
{
	FNProcGenEdMode::SetCellEdMode(FNProcGenEdMode::ENCellEdMode::CEM_Hull);
}

FSlateIcon FNProcGenEditorCommands::CellActorEditHullMode_GetIcon()
{
	if (FNProcGenEdMode::GetCellEdMode() == FNProcGenEdMode::ENCellEdMode::CEM_Hull)
	{
		return FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.Hull.Selected");
	}
	return FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.Hull");
}

void FNProcGenEditorCommands::CellActorEditBoundsMode()
{
	FNProcGenEdMode::SetCellEdMode(FNProcGenEdMode::ENCellEdMode::CEM_Bounds);
}

FSlateIcon FNProcGenEditorCommands::CellActorEditBoundsMode_GetIcon()
{
	if (FNProcGenEdMode::GetCellEdMode() == FNProcGenEdMode::ENCellEdMode::CEM_Bounds)
	{
		return FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.Bounds.Selected");
	}
	return FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.Bounds");
}

void FNProcGenEditorCommands::CellActorToggleDrawVoxelData()
{
	switch (FNProcGenEdMode::GetCellVoxelMode())
	{
	case FNProcGenEdMode::ENCellVoxelMode::CVM_Grid:
		FNProcGenEdMode::SetCellVoxelMode(FNProcGenEdMode::ENCellVoxelMode::CVM_Points);
		break;
	case FNProcGenEdMode::ENCellVoxelMode::CVM_None:
		FNProcGenEdMode::SetCellVoxelMode(FNProcGenEdMode::ENCellVoxelMode::CVM_Grid);
		break;
	default:
		FNProcGenEdMode::SetCellVoxelMode(FNProcGenEdMode::ENCellVoxelMode::CVM_None);
	}
}

FSlateIcon FNProcGenEditorCommands::CellActorToggleDrawVoxelData_GetIcon()
{
	switch (FNProcGenEdMode::GetCellVoxelMode())
	{
		case FNProcGenEdMode::ENCellVoxelMode::CVM_Grid:
			return FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.Voxel.Selected");
		case FNProcGenEdMode::ENCellVoxelMode::CVM_Points:
			return FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.Voxel.Points");
		default:
			return FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.Voxel");
	}
}

void FNProcGenEditorCommands::CellAddActor()
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
		FNProcGenEditorUtils::SaveCell(CurrentWorld, SpawnedActor);
	}
}

bool FNProcGenEditorCommands::CellAddActor_CanExecute()
{
	return !FNProcGenEdMode::HasCellActor();
}

bool FNProcGenEditorCommands::CellAddActor_CanShow()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;
	return FNProcGenEdMode::IsActive() && !FNProcGenEdMode::HasCellActor() && !FNProcGenEditorUtils::IsOrganComponentPresentInCurrentWorld();
}

void FNProcGenEditorCommands::OrganGenerate()
{
	// Create generator
	UNOrganGenerator* Generator = UNOrganGenerator::CreateInstance(FNProcGenEditorUtils::GetSelectedOrganComponents());
	Generator->Generate();
}

bool FNProcGenEditorCommands::OrganGenerate_CanExecute()
{
	return FNProcGenEditorUtils::IsOrganVolumeSelected();
}

void FNProcGenEditorCommands::CellSelectActor()
{
	GEditor->SelectNone(false, true);
	GEditor->SelectActor(FNProcGenEditorUtils::GetCellActorFromCurrentWorld(),
		true, true, true, true);
}

bool FNProcGenEditorCommands::CellSelectActor_CanExecute()
{
	return FNProcGenEdMode::HasCellActor() && GEditor->CanSelectActor(FNProcGenEdMode::GetCellActor(), false);
}

bool FNProcGenEditorCommands::CellSelectActor_CanShow()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;
	
	// Use cached version
	return FNProcGenEdMode::IsActive() && FNProcGenEdMode::HasCellActor();
}

void FNProcGenEditorCommands::CellCalculateAll()
{
	CellCalculateBounds();
	CellCalculateHull();
}

bool FNProcGenEditorCommands::CellCalculateAll_CanExecute()
{
	return FNProcGenEditorUtils::IsCellActorPresentInCurrentWorld();
}

void FNProcGenEditorCommands::CellCalculateBounds()
{
	ANCellActor* CellActor = FNProcGenEditorUtils::GetCellActorFromCurrentWorld();
	CellActor->CalculateBounds();
}

bool FNProcGenEditorCommands::CellCalculateBounds_CanExecute()
{
	return FNProcGenEditorUtils::IsCellActorPresentInCurrentWorld();
}

void FNProcGenEditorCommands::CellCalculateHull()
{
	ANCellActor* CellActor = FNProcGenEditorUtils::GetCellActorFromCurrentWorld();
	CellActor->CalculateHull();
}

bool FNProcGenEditorCommands::CellCalculateHull_CanExecute()
{
	return FNProcGenEditorUtils::IsCellActorPresentInCurrentWorld();
}

void FNProcGenEditorCommands::CellCalculateVoxelData()
{
	ANCellActor* CellActor = FNProcGenEditorUtils::GetCellActorFromCurrentWorld();
	CellActor->CalculateVoxelData();
}

bool FNProcGenEditorCommands::CellCalculateVoxelData_CanExecute()
{
	return FNProcGenEditorUtils::IsCellActorPresentInCurrentWorld();
}

void FNProcGenEditorCommands::CellResetCell()
{
	// Get the cell actor
	ANCellActor* CellActor = FNProcGenEditorUtils::GetCellActorFromCurrentWorld();
	CellActor->CellRoot->Reset();

	// Get the cell
	UNCell* Cell = UAssetDefinition_NCell::GetOrCreatePackage( FNEditorUtils::GetCurrentWorld());
	Cell->Root = FNCellRootDetails();
	Cell->Junctions.Empty();

	// Update the data
	FNProcGenEditorUtils::UpdateCell(Cell, CellActor);

	// Flag the actor as dirty
	CellActor->SetActorDirty();
}

bool FNProcGenEditorCommands::CellResetCell_CanExecute()
{
	return FNProcGenEditorUtils::IsCellActorPresentInCurrentWorld();
}

void FNProcGenEditorCommands::CellRemoveActor()
{
	UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
	ANCellActor* CellActor = FNProcGenUtils::GetCellActorFromWorld(CurrentWorld, true);
	CellActor->Destroy();
	// ReSharper disable once CppExpressionWithoutSideEffects
	CurrentWorld->MarkPackageDirty();
	
	// Need to destroy the sidecar.
	if (const FString SidecarPath = UAssetDefinition_NCell::GetCellPackagePath(CurrentWorld->GetOutermost()->GetName());
		FPackageName::DoesPackageExist(SidecarPath))
		{
			UEditorAssetLibrary::DeleteAsset(SidecarPath);
		}
}

bool FNProcGenEditorCommands::CellRemoveActor_CanExecute()
{
	return FNProcGenEditorUtils::IsCellActorPresentInCurrentWorld();
}

void FNProcGenEditorCommands::CellJunctionAddComponent()
{
	TArray<UNCellJunctionComponent*> OutComponents;
	TArray<UNCellJunctionComponent*> SelectComponents;
	bool bNeedsRefresh = false;
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		AActor* Actor = Cast<AActor>( *SelectedActor );
		if (!Actor) continue;
		
		UNCellJunctionComponent* NewComponent = static_cast<UNCellJunctionComponent*>(Actor->AddComponentByClass(
			UNCellJunctionComponent::StaticClass(), true, FTransform::Identity, false));
		
		
		NewComponent->AttachToComponent(Actor->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
		Actor->AddInstanceComponent(NewComponent);
		NewComponent->RegisterComponent();

		SelectComponents.Add(NewComponent);
		bNeedsRefresh = true;

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

bool FNProcGenEditorCommands::CellJunctionAddComponent_CanExecute()
{
	if (GEditor->GetSelectedActorCount() == 0) return false;
	return true;
}

void FNProcGenEditorCommands::CellJunctionSelectComponent(UNCellJunctionComponent* Junction)
{
	GEditor->SelectNone(false, true);
	GEditor->SelectComponent(Junction, true, true, true);
}

bool FNProcGenEditorCommands::CellJunctionSelectComponent_CanExecute(UNCellJunctionComponent* Junction)
{
	return true;
}

void FNProcGenEditorCommands::OrganSelectComponent(UNOrganComponent* Organ)
{
	GEditor->SelectNone(false, true);
	GEditor->SelectComponent(Organ, true, true, true);
}

bool FNProcGenEditorCommands::OrganSelectComponent_CanExecute(UNOrganComponent* Organ)
{
	return true;
}

void FNProcGenEditorCommands::CellCaptureThumbnail()
{
	if (FViewport* Viewport = GEditor->GetActiveViewport();
		ensure(GCurrentLevelEditingViewportClient) && ensure(Viewport) )
	{
		FLevelEditorViewportClient* OldViewportClient = GCurrentLevelEditingViewportClient;
		GCurrentLevelEditingViewportClient = nullptr;
		Viewport->Draw();

		if (const UWorld* World = FNEditorUtils::GetCurrentWorld();
			World != nullptr)
		{
			TArray<FAssetData> SelectedAssets;
			const FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
			FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(World));
			SelectedAssets.Emplace(AssetData);
			
			AssetViewUtils::CaptureThumbnailFromViewport(Viewport, SelectedAssets);
		}

		GCurrentLevelEditingViewportClient = OldViewportClient;
		Viewport->Draw();
	}
}

bool FNProcGenEditorCommands::CellCaptureThumbnail_CanExecute()
{
	const UWorld* World = FNEditorUtils::GetCurrentWorld();
	return World != nullptr && !FNEditorUtils::IsUnsavedWorld(World);
}

#undef LOCTEXT_NAMESPACE
