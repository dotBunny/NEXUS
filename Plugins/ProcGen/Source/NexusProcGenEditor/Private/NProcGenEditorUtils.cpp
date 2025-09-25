// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEditorUtils.h"

#include "AssetDefinitions/AssetDefinition_NCell.h"
#include "EditorAssetLibrary.h"
#include "NCoreEditorMinimal.h"
#include "Cell/NCell.h"
#include "Cell/NCellJunctionComponent.h"
#include "NEditorUtils.h"
#include "NProcGenUtils.h"
#include "Selection.h"
#include "Engine/Level.h"

bool FNProcGenEditorUtils::IsNCellActorPresentInCurrentWorld()
{
	if (const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld())
	{
		return FNProcGenUtils::GetNCellActorFromWorld(CurrentWorld, true) != nullptr;
	}
	return false;
}

ANCellActor* FNProcGenEditorUtils::GetNCellActorFromCurrentWorld()
{
	if (const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld())
	{
		return FNProcGenUtils::GetNCellActorFromWorld(CurrentWorld, true);
	}
	return nullptr;
}

bool FNProcGenEditorUtils::IsNCellActorSelected()
{
	for ( FSelectionIterator SelectedActor( GEditor->GetSelectedActorIterator() ) ; SelectedActor ; ++SelectedActor )
	{
		if (Cast<ANCellActor>( *SelectedActor )) return true;
	}
	return false;
}

void FNProcGenEditorUtils::SaveNCell(UWorld* World, ANCellActor* CellActor)
{
	if (CellActor == nullptr)
	{
		CellActor = FNProcGenUtils::GetNCellActorFromWorld(World, true);
	}

	// Last chance
	if (CellActor == nullptr)
	{
		NE_LOG(Warning, TEXT("[FNProcGenEditorUtils::SaveNCell] No NCellActor found in the world when trying to save NCell."));
		return;
	}
	
	UNCell* Cell = UAssetDefinition_NCell::GetOrCreatePackage(World);
	if (UpdateNCell(Cell, CellActor))
	{
		// Need to tell the cell it's dirty so it gets saved to disk
		// ReSharper disable once CppExpressionWithoutSideEffects
		Cell->MarkPackageDirty();
		UEditorAssetLibrary::SaveLoadedAsset(Cell);
	}
	else
	{
		NE_LOG(Log, TEXT("[FNProcGenEditorUtils::SaveNCell] No updates made."));
	}
}

bool FNProcGenEditorUtils::UpdateNCell(UNCell* Cell, ANCellActor* CellActor)
{
	bool bUpdatedCellData = false;

	// Update Our Cell Overall Data (in the level, not copied at this point)
	if (CellActor->CellRoot->Details.BoundsSettings.bCalculateOnSave)
	{
		CellActor->CalculateBounds();
	}

	if (CellActor->CellRoot->Details.HullSettings.bCalculateOnSave)
	{
		CellActor->CalculateHull();
	}
	
	// Apply actor root data to the NCell root cache
	if (!CellActor->CellRoot->Details.IsEqual(Cell->Root))
	{
		CellActor->CellRoot->Details.CopyTo(Cell->Root);
		bUpdatedCellData = true;
	}

	
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
	
	// Ensure the Cell is mapped to the component.
	if (CellActor->Sidecar != Cell)
	{
		CellActor->Sidecar = Cell;

		// We changed it, make sure the level is known dirty too.
		if (!CellActor->MarkPackageDirty())
		{
			NE_LOG(Warning, TEXT("[FNProcGenEditorUtils::UpdateNCell] Failed to mark package dirty for CellActor in %s."), *CellActor->GetWorld()->GetName());
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