// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorUtils.h"

#include "AssetDefinitions/AssetDefinition_NCell.h"
#include "EditorAssetLibrary.h"
#include "Cell/NCell.h"
#include "Cell/NCellJunctionComponent.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyEditorMinimal.h"
#include "NWorldAssemblyEditorSettings.h"
#include "NWorldAssemblyEditorSubsystem.h"
#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblyRegistry.h"
#include "NWorldAssemblyUtils.h"
#include "Selection.h"
#include "Engine/Level.h"
#include "Assembly/Contexts/NVirtualWorldContext.h"
#include "Assembly/Tasks/NCreateVirtualWorldTask.h"
#include "Macros/NFlagsMacros.h"
#include "Organ/NOrganVolume.h"
#include "Types/NRawMeshFactory.h"
#include "Types/NRawMeshUtils.h"

ANDebugActor* FNWorldAssemblyEditorUtils::CreateWorldCollisionVisualizerActor(UWorld* World, const TArray<FBoxSphereBounds>& Bounds)
{
	const TArray<AActor*> WorldActors = FNActorUtils::GetWorldActors(World, FNCreateVirtualWorldTask::CreateWorldActorFilterSettings());
	
	TArray<FNRawMesh> WorldCollisionMeshes;
	TArray<FTransform> WorldCollisionMeshTransforms;
	FNRawMeshFactory::FromActorsInBounds(WorldActors, Bounds, WorldCollisionMeshes, WorldCollisionMeshTransforms);
	
	UMaterialInterface* VisualizerMaterial = UNWorldAssemblyEditorSettings::Get()->CollisionVisualizerMaterial.LoadSynchronous();
	if (VisualizerMaterial == nullptr) return nullptr;
	
	TArray<ANDebugActor*> Actors = FNRawMeshUtils::CreateRawMeshVisualizers(World, WorldCollisionMeshes, WorldCollisionMeshTransforms, 
		VisualizerMaterial, true, false);
	
	if (Actors.IsEmpty()) return nullptr;
	
	return Actors[0];
}

bool FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld()
{
	if (const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld())
	{
		return FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true) != nullptr;
	}
	return false;
}

ANCellActor* FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld()
{
	if (const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld())
	{
		return FNWorldAssemblyUtils::GetCellActorFromWorld(CurrentWorld, true);
	}
	return nullptr;
}

bool FNWorldAssemblyEditorUtils::IsCellActorSelected()
{
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		if (Cast<ANCellActor>( *SelectedActor )) return true;
	}
	return false;
}

TArray<ANCellActor*> FNWorldAssemblyEditorUtils::GetSelectedCellActors()
{
	TArray<ANCellActor*> Result;
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		if (ANCellActor* TestActor = Cast<ANCellActor>( *SelectedActor )) Result.Add(TestActor);
	}
	return MoveTemp(Result);
}

bool FNWorldAssemblyEditorUtils::IsOrganVolumeSelected()
{
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		if (Cast<ANOrganVolume>( *SelectedActor )) return true;
	}
	return false;
}

bool FNWorldAssemblyEditorUtils::IsOrganComponentPresentInCurrentWorld()
{
	if (const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld())
	{
		return FNWorldAssemblyRegistry::HasOrganComponentsInWorld(CurrentWorld);
	}
	return false;
}

TArray<ANOrganVolume*> FNWorldAssemblyEditorUtils::GetSelectedOrganVolumes(const bool bSorted)
{
	TArray<ANOrganVolume*> Result;
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		if (ANOrganVolume* TestVolume = Cast<ANOrganVolume>( *SelectedActor )) Result.Add(TestVolume);
	}
	
	if (bSorted)
	{
		Result.Sort([](const ANOrganVolume& A, const ANOrganVolume& B) {
			return A.GetOrganComponent()->Identifier < B.GetOrganComponent()->Identifier;
		});
	}
	return MoveTemp(Result);
}

TArray<UNOrganComponent*> FNWorldAssemblyEditorUtils::GetSelectedOrganComponents(const bool bSorted)
{
	TArray<UNOrganComponent*> Components;
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		if (const ANOrganVolume* Organ = Cast<ANOrganVolume>( *SelectedActor ))
		{
			Components.Add(Organ->GetOrganComponent());
		}
	}
	
	if (bSorted)
	{
		Components.Sort([](const UNOrganComponent& A, const UNOrganComponent& B) {
			return A.Identifier < B.Identifier;
		});
	}
	
	return MoveTemp(Components);
}

ENWorldAssemblySelectionFlags FNWorldAssemblyEditorUtils::GetSelectionFlags()
{
	uint8 Flags = 0;
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		if (Cast<ANCellActor>( *SelectedActor ))
		{
			N_FLAGS_ADD_UINT8(Flags, ENWorldAssemblySelectionFlags::CellActor);
		}
		
		if (Cast<ANOrganVolume>( *SelectedActor ))
		{
			N_FLAGS_ADD_UINT8(Flags, ENWorldAssemblySelectionFlags::OrganVolume);
		}
	}
	return static_cast<ENWorldAssemblySelectionFlags>(Flags);
}

bool FNWorldAssemblyEditorUtils::HasGeneratedCellProxies()
{
	return UNWorldAssemblyEditorSubsystem::Get()->HasGeneratedCellProxies();
}

bool FNWorldAssemblyEditorUtils::HasSelectedGeneratedCellProxies()
{
	TArray<UNOrganComponent*> OrganComponents = FNWorldAssemblyEditorUtils::GetSelectedOrganComponents();
	for (UNOrganComponent* OrganComponent : OrganComponents)
	{
		if (OrganComponent->GetLastOperationTicket() != 0)
		{
			return true;
		}
	}
	return false;
}

void FNWorldAssemblyEditorUtils::SaveCell(UWorld* World, ANCellActor* CellActor, bool bForceSave)
{
	if (CellActor == nullptr)
	{
		CellActor = FNWorldAssemblyUtils::GetCellActorFromWorld(World, true);
	}

	// Last chance
	if (CellActor == nullptr)
	{
		UE_LOG(LogNexusWorldAssemblyEditor, Warning, TEXT("No ANCellActor found in the world when trying to save UNCell."));
		return;
	}

	if (UNCell* Cell = UAssetDefinition_NCell::GetOrCreatePackage(World); 
		(UpdateCell(Cell, CellActor) || bForceSave))
	{
		// Need to tell the cell it's dirty so it gets saved to disk
		// ReSharper disable once CppExpressionWithoutSideEffects
		Cell->MarkPackageDirty();
		UEditorAssetLibrary::SaveLoadedAsset(Cell);
	}
}

bool FNWorldAssemblyEditorUtils::UpdateCell(UNCell* Cell, ANCellActor* CellActor)
{
	bool bUpdatedCellData = false;
	
	FScopedSlowTask MainTask = FScopedSlowTask(7, NSLOCTEXT("NexusWorldAssemblyEditor", "Task_UpdateCell", "Update Cell"));
	MainTask.MakeDialog(false);

	// STEP 1 - Ensure CellActor Setup
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusWorldAssemblyEditor", "Task_UpdateCell_Step1", "CellActor Setup ..."));
	FString CellActorName = TEXT("NCellActor__");
	CellActorName.Append( FPackageName::GetShortName(CellActor->GetWorld()->GetOutermost()->GetName()));
	if (!CellActor->GetActorLabel().Equals(CellActorName))
	{
		CellActor->SetActorLabel(CellActorName);
	}
	
	// STEP 2 - Calculate Bounds
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusWorldAssemblyEditor", "Task_UpdateCell_Step2", "Cell Bounds ..."));
	// Update Our Cell Overall Data (in the level, not copied at this point)
	if (CellActor->CellRoot->Details.BoundsSettings.bCalculateOnSave)
	{
		CellActor->CalculateBounds();
	}

	// STEP 3 - Calculate Hull
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusWorldAssemblyEditor", "Task_UpdateCell_Step3", "Cell Hull ..."));
	if (CellActor->CellRoot->Details.HullSettings.bCalculateOnSave)
	{
		CellActor->CalculateHull();
		FNWorldAssemblyEdMode::ProtectCellEdMode();
	}
	
	// STEP 4 - Calculate Voxel Data
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusWorldAssemblyEditor", "Task_UpdateCell_Step4", "Cell Voxel ..."));
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
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusWorldAssemblyEditor", "Task_UpdateCell_Step5", "Apply Actor Root Data ..."));
	if (!CellActor->CellRoot->Details.IsEqual(Cell->Root))
	{
		CellActor->Modify();
		CellActor->CellRoot->Details.CopyTo(Cell->Root);
		bUpdatedCellData = true;
	}
	
	// STEP 6 - Clean up Junction Data
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusWorldAssemblyEditor", "Task_UpdateCell_Step6", "Clean Up Junction Data ..."));
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
		for (int32 i = 0; i < ToRemove.Num(); i++)
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
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusWorldAssemblyEditor", "Task_UpdateCell_Step7", "Ensure Sidecar ..."));
	
	// Ensure the Cell is mapped to the component.
	if (CellActor->Sidecar != Cell)
	{
		CellActor->Sidecar = Cell;

		// We changed it, make sure the level is known dirty too.
		if (!CellActor->MarkPackageDirty())
		{
			UE_LOG(LogNexusWorldAssemblyEditor, Warning, TEXT("Failed to mark UPackage dirty for ANCellActor(%s) in UWorld(%s) when updating UNCell(%s)."), 
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
	
	return bUpdatedCellData;
}