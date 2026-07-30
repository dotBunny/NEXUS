// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorUtils.h"

#include "AssetDefinitions/AssetDefinition_NCell.h"
#include "EditorAssetLibrary.h"
#include "EngineUtils.h"
#include "Cell/NCell.h"
#include "Cell/NCellJunctionComponent.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyEditorMinimal.h"
#include "NWorldAssemblyEditorSettings.h"
#include "NWorldAssemblyEditorSubsystem.h"
#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblyRegistry.h"
#include "NWorldAssemblyUtils.h"
#include "NWorldCollisionCache.h"
#include "Selection.h"
#include "Engine/Level.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Macros/NFlagsMacros.h"
#include "Organ/NOrganVolume.h"

ANDebugActor* FNWorldAssemblyEditorUtils::RefreshWorldCollisionVisualizerActor(UWorld* World, const TArray<FBoxSphereBounds>& Bounds,
	ANDebugActor* ExistingActor, TArray<AActor*>& OutSourceActors)
{
	// Single producer of the merged world-collision mesh: shared with the bone penetration readout via
	// FNWorldCollisionCache. Build also reports the source actors so the ed mode can track relevance for refreshes.
	const FNRawMesh MergedMesh = FNWorldCollisionCache::Build(World, Bounds, &OutSourceActors);

	UMaterialInterface* VisualizerMaterial = UNWorldAssemblyEditorSettings::Get()->CollisionVisualizerMaterial.LoadSynchronous();

	// Refresh path: swap the merged geometry onto the live actor (possibly emptying it) without re-spawning.
	if (ExistingActor != nullptr)
	{
		ExistingActor->OverrideWithDynamicMesh(MergedMesh.CreateDynamicMesh(false), VisualizerMaterial);
		return ExistingActor;
	}

	// Initial build: nothing to show, or no material configured — don't spawn anything.
	if (MergedMesh.Loops.Num() == 0 || VisualizerMaterial == nullptr) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = MakeUniqueObjectName(World, ANDebugActor::StaticClass(), FName("NWorldCollisionVisualizer"));
	FString ActorLabel = SpawnParams.Name.ToString();
#if WITH_EDITOR
	SpawnParams.InitialActorLabel = ActorLabel;
#endif // WITH_EDITOR
	SpawnParams.ObjectFlags |= RF_Transient;

	ANDebugActor* DebugActor = World->SpawnActor<ANDebugActor>(ANDebugActor::StaticClass(), FTransform::Identity, SpawnParams);
	if (DebugActor == nullptr) return nullptr;

	DebugActor->OverrideWithDynamicMesh(MergedMesh.CreateDynamicMesh(false), VisualizerMaterial);
	return DebugActor;
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
	return Result;
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
	return Result;
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

	return Components;
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

UNCell* FNWorldAssemblyEditorUtils::SyncCell(UWorld* World, ANCellActor* CellActor, bool bForceSave)
{
	if (CellActor == nullptr)
	{
		CellActor = FNWorldAssemblyUtils::GetCellActorFromWorld(World, true);
	}

	// Last chance
	if (CellActor == nullptr)
	{
		UE_LOG(LogNexusWorldAssemblyEditor, Warning, TEXT("No ANCellActor found in the world when trying to save UNCell."));
		return nullptr;
	}

	UNCell* Cell = UAssetDefinition_NCell::GetOrCreatePackage(World);
	if (Cell == nullptr)
	{
		UE_LOG(LogNexusWorldAssemblyEditor, Warning, TEXT("Unable to get or create the UNCell side-car package when trying to save."));
		return nullptr;
	}

	if (UpdateCell(Cell, CellActor) || bForceSave)
	{
		// Need to tell the cell it's dirty so it gets saved to disk
		// ReSharper disable once CppExpressionWithoutSideEffects
		Cell->MarkPackageDirty();
		return Cell;
	}

	return nullptr;
}

void FNWorldAssemblyEditorUtils::SaveCell(UWorld* World, ANCellActor* CellActor, bool bForceSave)
{
	// Sync the cell data into its side-car (in-memory) and, when something changed, write it straight to disk. This is the
	// synchronous path used by explicit user actions (Save Cell menu, cell spawn, commandlet) outside the world-save flow.
	if (UNCell* Cell = SyncCell(World, CellActor, bForceSave))
	{
		if (!UEditorAssetLibrary::SaveLoadedAsset(Cell))
		{
			// The data is synced in memory and the package left dirty; surface the disk-write failure so a source-control
			// problem (not checked out, exclusively locked, offline) isn't missed — especially in the headless commandlet.
			UE_LOG(LogNexusWorldAssemblyEditor, Warning,
				TEXT("Failed to write the UNCell side-car '%s' to disk. It may not be checked out in source control; the change is kept in memory."),
				*Cell->GetName());
		}
	}
}

void FNWorldAssemblyEditorUtils::EnsureCellInitializedCallbackActors(const UWorld* World, ANCellActor* CellActor)
{
	TArray<TObjectPtr<AActor>> FoundActors;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (IsValid(Actor) && Actor->Implements<UNCellInitialized>())
		{
			FoundActors.Add(Actor);
		}
	}
	if (!FNArrayUtils::IsSameUnorderedValues<TObjectPtr<AActor>>(CellActor->CellInitializedTargets, FoundActors))
	{
		CellActor->CellInitializedTargets = FoundActors;
		// ReSharper disable once CppExpressionWithoutSideEffects
		CellActor->MarkPackageDirty();
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
		// Junction Details
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
			bUpdatedCellData =  true;
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

bool FNWorldAssemblyEditorUtils::CanGenerateSelectedOrgan()
{
	if (FNEditorUtils::IsNotPlayInEditor() && IsOrganVolumeSelected() && !FNWorldAssemblyRegistry::HasOperations())
	{
		return true;
	}
	return false;
}

bool FNWorldAssemblyEditorUtils::CanGenerateAllOrgans()
{
	if (FNEditorUtils::IsNotPlayInEditor() && FNWorldAssemblyRegistry::HasOrganComponents() && !FNWorldAssemblyRegistry::HasOperations())
	{
		return true;
	}
	return false;
}

TArray<FAssetData> FNWorldAssemblyEditorUtils::GetAllCellDataAssetData(bool bWaitForFullScan)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	if (bWaitForFullScan)
	{
		AssetRegistry.SearchAllAssets(true);
	}

	FARFilter Filter;
	Filter.ClassPaths.Add(UNCell::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;

	TArray<FAssetData> FoundAssets;
	AssetRegistry.GetAssets(Filter, FoundAssets);
	return FoundAssets;
}
