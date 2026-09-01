// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorUtils.h"

#include "AssetDefinitions/AssetDefinition_NCell.h"
#include "EditorAssetLibrary.h"
#include "EngineUtils.h"
#include "Cell/NCell.h"
#include "Cell/NCellJunctionComponent.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyEditorColors.h"
#include "NWorldAssemblyEditorMinimal.h"
#include "NWorldAssemblyEditorSettings.h"
#include "NWorldAssemblyEditorSubsystem.h"
#include "EdMode/NWorldAssemblyEdMode.h"
#include "NWorldAssemblyRegistry.h"
#include "NWorldAssemblySettings.h"
#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblyUtils.h"
#include "NWorldCollisionBaker.h"
#include "NWorldCollisionCacheSave.h"
#include "NWorldCollisionPreview.h"
#include "ScopedTransaction.h"
#include "Selection.h"
#include "Engine/Level.h"
#include "Misc/ScopedSlowTask.h"
#include "Organ/NOrganComponent.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Macros/NFlagsMacros.h"
#include "Organ/NOrganVolume.h"

ANDebugActor* FNWorldAssemblyEditorUtils::RefreshWorldCollisionVisualizerActor(UWorld* World, ANDebugActor* ExistingActor)
{
	// The level's baked collision, shared with the bone penetration readout through FNWorldCollisionPreview. Nothing
	// is gathered: what is drawn is the geometry an assembly will collide against, not a second derivation of it.
	//
	// Bounds are gone from this signature along with the gather — the pool is scoped by the organs that reference it,
	// which is the same thing the caller used to express by passing none.
	const FNRawMesh& MergedMesh = FNWorldCollisionPreview::GetMesh(World);

	UMaterialInterface* VisualizerMaterial = UNWorldAssemblyEditorSettings::Get()->CollisionVisualizerMaterial.LoadSynchronous();

	// A stale preview keeps drawing the last baked state, which is the right call while authoring but leaves the
	// visualizer looking exactly like a current one — the single most dangerous thing it can do, because the whole
	// reason to spawn it is to trust what it shows. The wireframe overlay is the difference, and it is on the actor
	// itself rather than in a notification: it is true for as long as it is true, and it is visible from wherever the
	// user is already looking.
	const bool bStale = FNWorldCollisionPreview::GetState(World) != FNWorldCollisionPreview::EState::Available;

	// Refresh path: swap the merged geometry onto the live actor (possibly emptying it) without re-spawning.
	if (ExistingActor != nullptr)
	{
		ExistingActor->OverrideWithDynamicMesh(MergedMesh.CreateDynamicMesh(false), VisualizerMaterial);
		MarkWorldCollisionVisualizerStale(ExistingActor, bStale);
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
	MarkWorldCollisionVisualizerStale(DebugActor, bStale);
	return DebugActor;
}

void FNWorldAssemblyEditorUtils::MarkWorldCollisionVisualizerStale(const ANDebugActor* VisualizerActor, const bool bStale)
{
	UDynamicMeshComponent* DynamicMesh = VisualizerActor->GetDynamicMeshComponent();
	if (DynamicMesh == nullptr) return;

	// The component's own wireframe pass rather than a second material, so this needs no art and cannot fall back to
	// an unlit magenta the way a missing asset would. Set unconditionally so that clearing it is the same code path
	// as setting it, and a bake takes the overlay away without anyone having to remember to.
	DynamicMesh->WireframeColor = FNWorldAssemblyEditorColors::GetWorldCollisionStale();
	DynamicMesh->SetEnableWireframeRenderPass(bStale);
	DynamicMesh->MarkRenderStateDirty();
}

void FNWorldAssemblyEditorUtils::CacheWorldCollision(UWorld* World, const TArray<UNOrganComponent*>& Organs)
{
	if (World == nullptr) return;

	const FNWorldAssemblyWorldCollisionSettings& Settings = UNWorldAssemblySettings::Get()->WorldCollisionSettings;

	// The bake writes to the organ components and to the level's cache actor, so it is transacted like any other
	// authoring action.
	const FScopedTransaction Transaction(
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorUtils_CacheWorldCollision", "Cache World Collision"));

	const FNWorldCollisionBaker::FBakeResult Result = Organs.IsEmpty()
		? FNWorldCollisionBaker::BakeWorld(World, Settings, true)
		: FNWorldCollisionBaker::BakeOrgans(World, Organs, Settings, true);

	// Every organ has just been fingerprinted against the live world, so the save-time pass has nothing left to find.
	// Only sound for a whole-level bake that ran to completion — a selected-organ bake leaves the rest of the level
	// unexamined, and a cancelled one leaves the organs it never reached unexamined too.
	if (Organs.IsEmpty() && !Result.bCancelled)
	{
		FNWorldCollisionCacheSave::MarkClean(World);
	}

	// Reported rather than silent: the bake's whole value is that it moves work off the assembly, and the only way to
	// see it happened is to say so. A run that changed nothing is worth saying too — it means the level is unchanged.
	UE_LOG(LogNexusWorldAssembly, Log, TEXT("World collision cache: baked %d organ(s)%s%s."),
		Result.OrgansBaked,
		Result.bChanged ? TEXT("") : TEXT(" (nothing changed)"),
		Result.bCancelled ? TEXT(" - cancelled before finishing") : TEXT(""));
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

bool FNWorldAssemblyEditorUtils::CanEditCell()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;
	return IsCellActorPresentInCurrentWorld();
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

	// Last-resort guard, not the main defense. Every path that can wait already has by the time it reaches here —
	// the Calculate commands, Force Save and the commandlet all call FNTerrainUtils::WaitForSettle first. What is left is a
	// world save started from outside this module (Ctrl+S), which reaches UpdateCell through
	// UAssetDefinition_NCell::OnPreSaveWorldWithContext, already inside UEditorEngine::SavePackage — where waiting
	// would mean ticking the editor mid-save. So this one case declines to recalculate rather than baking a snapshot
	// of a half-built terrain: a save is silent, what it writes is what ships, and leaving the previous values means
	// an out-of-date cell, which the side-car diff already reports.
	const bool bTerrainSettled = UNWorldAssemblyEdMode::IsTerrainSettled();
	if (!bTerrainSettled)
	{
		UE_LOG(LogNexusWorldAssemblyEditor, Warning,
			TEXT("Skipping save-time recalculation for '%s': its terrain is still building. Existing cell data has been left alone; use Calculate once the terrain settles, which waits for it."),
			*CellActor->GetActorLabel());
	}

	// STEP 2 - Calculate Bounds
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusWorldAssemblyEditor", "Task_UpdateCell_Step2", "Cell Bounds ..."));
	// Update Our Cell Overall Data (in the level, not copied at this point)
	if (bTerrainSettled && CellActor->CellRoot->Details.BoundsSettings.bCalculateOnSave)
	{
		CellActor->CalculateBounds();
	}

	// STEP 3 - Calculate Hull
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusWorldAssemblyEditor", "Task_UpdateCell_Step3", "Cell Hull ..."));
	if (bTerrainSettled && CellActor->CellRoot->Details.HullSettings.bCalculateOnSave)
	{
		CellActor->CalculateHull();
		UNWorldAssemblyEdMode::ProtectCellEdMode();
	}

	// STEP 4 - Calculate Voxel Data
	MainTask.EnterProgressFrame(1, NSLOCTEXT("NexusWorldAssemblyEditor", "Task_UpdateCell_Step4", "Cell Voxel ..."));
	if (bTerrainSettled && CellActor->CellRoot->Details.VoxelSettings.bCalculateOnSave)
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
