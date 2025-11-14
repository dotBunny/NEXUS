// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEditorUndo.h"

#include "NProcGenEditorUtils.h"
#include "Cell/NCellJunctionComponent.h"
#include "NProcGenRegistry.h"
#include "Cell/NCell.h"

void FNProcGenEditorUndo::PostUndo(bool bSuccess)
{
	UpdateKnownJunctions();
	CompareAgainstSidecar();
}

void FNProcGenEditorUndo::PostRedo(bool bSuccess)
{
	UpdateKnownJunctions();
	CompareAgainstSidecar();
}

void FNProcGenEditorUndo::UpdateKnownJunctions()
{
	for (const auto Junction : FNProcGenRegistry::GetCellJunctionComponents())
	{
		// We want to make sure our relative calculations are updated in this situation
		Junction->OnTransformUpdated(nullptr, EUpdateTransformFlags::None, ETeleportType::None);
	}
}

void FNProcGenEditorUndo::CompareAgainstSidecar()
{
	for (const auto Root : FNProcGenRegistry::GetCellRootComponents())
	{
		// Do not process undo on proxy-based actors
		if (ANCellActor* CellActor = Root->GetNCellActor(); 
			CellActor != nullptr && !CellActor->WasSpawnedFromProxy()) 
		{
			CellActor->Modify();
			CellActor->SetActorDirty(CellActor->HasDifferencesFromSidecar());
		}
	}
}

bool FNProcGenEditorUndo::MatchesContext(const FTransactionContext& InContext,
                                         const TArray<TPair<UObject*, FTransactionObjectEvent>>& TransactionObjectContexts) const
{
	for (const auto Pair : TransactionObjectContexts)
	{
		// Actor
		if (const AActor* Actor = Cast<AActor>(Pair.Key);
			Actor != nullptr)
		{
			if (FNProcGenEditorUtils::EffectsGeneratedData(Actor)) return true;
		}

		// Component
		if (const UActorComponent* Component = Cast<UActorComponent>(Pair.Key);
			Component != nullptr)
		{
			
			if (FNProcGenEditorUtils::EffectsGeneratedData(Component)) return true;
			if (FNProcGenEditorUtils::EffectsGeneratedData(Component->GetOwner())) return true;
		}
	}
	
	return false;
}
