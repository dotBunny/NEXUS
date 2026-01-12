// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEditorUtils.h"

#include "AssetDefinitions/AssetDefinition_NCell.h"
#include "EditorAssetLibrary.h"
#include "Cell/NCell.h"
#include "Cell/NCellJunctionComponent.h"
#include "NEditorUtils.h"
#include "NProcGenEditorMinimal.h"
#include "NProcGenEdMode.h"
#include "NProcGenRegistry.h"
#include "NProcGenUtils.h"
#include "Selection.h"
#include "Engine/Level.h"
#include "Macros/NFlagsMacros.h"
#include "Organ/NOrganVolume.h"

bool FNProcGenEditorUtils::IsCellActorPresentInCurrentWorld()
{
	if (const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld())
	{
		return FNProcGenUtils::GetCellActorFromWorld(CurrentWorld, true) != nullptr;
	}
	return false;
}

ANCellActor* FNProcGenEditorUtils::GetCellActorFromCurrentWorld()
{
	if (const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld())
	{
		return FNProcGenUtils::GetCellActorFromWorld(CurrentWorld, true);
	}
	return nullptr;
}

bool FNProcGenEditorUtils::IsCellActorSelected()
{
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		if (Cast<ANCellActor>( *SelectedActor )) return true;
	}
	return false;
}

TArray<ANCellActor*> FNProcGenEditorUtils::GetSelectedCellActors()
{
	TArray<ANCellActor*> Result;
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		if (ANCellActor* TestActor = Cast<ANCellActor>( *SelectedActor )) Result.Add(TestActor);
	}
	return MoveTemp(Result);
}

bool FNProcGenEditorUtils::IsOrganVolumeSelected()
{
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		if (Cast<ANOrganVolume>( *SelectedActor )) return true;
	}
	return false;
}

bool FNProcGenEditorUtils::IsOrganComponentPresentInCurrentWorld()
{
	if (const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld())
	{
		return FNProcGenRegistry::HasOrganComponentsInWorld(CurrentWorld);
	}
	return false;
}

TArray<ANOrganVolume*> FNProcGenEditorUtils::GetSelectedOrganVolumes()
{
	TArray<ANOrganVolume*> Result;
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		if (ANOrganVolume* TestVolume = Cast<ANOrganVolume>( *SelectedActor )) Result.Add(TestVolume);
	}
	return MoveTemp(Result);
}

TArray<UNOrganComponent*> FNProcGenEditorUtils::GetSelectedOrganComponents()
{
	TArray<UNOrganComponent*> Components;
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		if (const ANOrganVolume* Organ = Cast<ANOrganVolume>( *SelectedActor ))
		{
			Components.Add(Organ->GetOrganComponent());
		}
	}
	return MoveTemp(Components);
}

ENProcGenSelectionFlags FNProcGenEditorUtils::GetSelectionFlags()
{
	uint8 Flags = 0;
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		if (Cast<ANCellActor>( *SelectedActor ))
		{
			N_FLAGS_ADD(Flags, PGSF_CellActor);
		}
		
		if (Cast<ANOrganVolume>( *SelectedActor ))
		{
			N_FLAGS_ADD(Flags, PGSF_OrganVolume);
		}
	}
	return static_cast<ENProcGenSelectionFlags>(Flags);
}

void FNProcGenEditorUtils::SaveCell(UWorld* World, ANCellActor* CellActor)
{
	if (CellActor == nullptr)
	{
		CellActor = FNProcGenUtils::GetCellActorFromWorld(World, true);
	}

	// Last chance
	if (CellActor == nullptr)
	{
		UE_LOG(LogNexusProcGenEditor, Warning, TEXT("No ANCellActor found in the world when trying to save UNCell."));
		return;
	}

	if (UNCell* Cell = UAssetDefinition_NCell::GetOrCreatePackage(World); 
		UpdateCell(Cell, CellActor))
	{
		// Need to tell the cell it's dirty so it gets saved to disk
		// ReSharper disable once CppExpressionWithoutSideEffects
		Cell->MarkPackageDirty();
		UEditorAssetLibrary::SaveLoadedAsset(Cell);
	}
}

bool FNProcGenEditorUtils::UpdateCell(UNCell* Cell, ANCellActor* CellActor)
{
	bool bUpdatedCellData = false;
	
	FScopedSlowTask MainTask = FScopedSlowTask(7, NSLOCTEXT("NexusProcGenEditor", "Task_UpdateCell", "Update Cell"));
	MainTask.MakeDialog(false);

	// STEP 1 - Ensure CellActor Setup
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusProcGenEditor", "Task_UpdateCell_Step1", "CellActor Setup ..."));
	FString CellActorName = TEXT("NCellActor__");
	CellActorName.Append( FPackageName::GetShortName(CellActor->GetWorld()->GetOutermost()->GetName()));
	if (!CellActor->GetActorLabel().Equals(CellActorName))
	{
		CellActor->SetActorLabel(CellActorName);
	}
	
	// STEP 2 - Calculate Bounds
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusProcGenEditor", "Task_UpdateCell_Step2", "Cell Bounds ..."));
	// Update Our Cell Overall Data (in the level, not copied at this point)
	if (CellActor->CellRoot->Details.BoundsSettings.bCalculateOnSave)
	{
		CellActor->CalculateBounds();
	}

	// STEP 3 - Calculate Hull
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusProcGenEditor", "Task_UpdateCell_Step3", "Cell Hull ..."));
	if (CellActor->CellRoot->Details.HullSettings.bCalculateOnSave)
	{
		CellActor->CalculateHull();
		FNProcGenEdMode::ProtectCellEdMode();
	}
	
	// STEP 4 - Calculate Voxel Data
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusProcGenEditor", "Task_UpdateCell_Step4", "Cell Voxel ..."));
	if (CellActor->CellRoot->Details.VoxelSettings.bCalculateOnSave)
	{
		CellActor->CalculateVoxelData();
	}
	
	// STEP 4A - Clear Data If Not Suppose To Be There
	if (!CellActor->CellRoot->Details.VoxelSettings.bUseVoxelData && CellActor->CellRoot->Details.VoxelData.GetCount() != 0)
	{
		CellActor->Modify();
		CellActor->CellRoot->Details.VoxelData = FNCellVoxelData();
		CellActor->SetActorDirty();
	}

	// STEP 5 - Apply actor root data to the NCell root cache
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusProcGenEditor", "Task_UpdateCell_Step5", "Apply Actor Root Data ..."));
	if (!CellActor->CellRoot->Details.IsEqual(Cell->Root))
	{
		CellActor->Modify();
		CellActor->CellRoot->Details.CopyTo(Cell->Root);
		bUpdatedCellData = true;
	}
	
	// STEP 6 - Clean up Junction Data
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusProcGenEditor", "Task_UpdateCell_Step6", "Clean Up Junction Data ..."));
	const TMap<int32, TObjectPtr<UNCellJunctionComponent>>& JunctionComponents = CellActor->CellJunctions;

	// Clear out old data
	TArray<int32> ToRemove;
	for (const TPair<int32, FNCellJunctionDetails>& JunctionPair : Cell->Junctions)
	{
		if (!JunctionComponents.Contains(JunctionPair.Key))
		{
			ToRemove.Add(JunctionPair.Key);
		}
	}
	if (ToRemove.Num() > 0)
	{
		for (int i = 0; i < ToRemove.Num(); i++)
		{
			Cell->Junctions.Remove(ToRemove[i]);
		}
		bUpdatedCellData = true;
	}


	for (const TPair<int32, TObjectPtr<UNCellJunctionComponent>>& JunctionPair : JunctionComponents)
	{
		if (Cell->Junctions.Contains(JunctionPair.Key) )
		{
			if (!Cell->Junctions[JunctionPair.Key].IsEqual(JunctionPair.Value->Details))
			{
				JunctionPair.Value->Details.CopyTo(Cell->Junctions[JunctionPair.Key]);
				bUpdatedCellData = true;
			}
		}
		else
		{
			Cell->Junctions.Add(JunctionPair.Key, JunctionPair.Value->Details);
		}
	}
	
	// STEP 7 - Ensure Sidecar
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusProcGenEditor", "Task_UpdateCell_Step7", "Ensure Sidecar ..."));
	
	// Ensure the Cell is mapped to the component.
	if (CellActor->Sidecar != Cell)
	{
		CellActor->Sidecar = Cell;

		// We changed it, make sure the level is known dirty too.
		if (!CellActor->MarkPackageDirty())
		{
			UE_LOG(LogNexusProcGenEditor, Warning, TEXT("Failed to mark UPackage dirty for ANCellActor(%s) in UWorld(%s) when updating UNCell(%s)."), 
				*CellActor->GetName(), *CellActor->GetWorld()->GetName(), *Cell->GetName());
		}
		bUpdatedCellData = true;
	}

	// We've synced things by this point
	CellActor->bActorDirty = false;

	// This is the only place that will update the version
	if (bUpdatedCellData)
	{
		Cell->Version++;
	}
	
	MainTask.Destroy();
	
	return bUpdatedCellData;
}