// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorUndo.h"

#include "NWorldAssemblyEditorUtils.h"
#include "Cell/NCellJunctionComponent.h"
#include "NWorldAssemblyRegistry.h"
#include "Cell/NCell.h"

void FNWorldAssemblyEditorUndo::PostUndo(bool bSuccess)
{
	UpdateKnownJunctions();
	CompareAgainstSidecar();
}

void FNWorldAssemblyEditorUndo::PostRedo(bool bSuccess)
{
	UpdateKnownJunctions();
	CompareAgainstSidecar();
}

void FNWorldAssemblyEditorUndo::UpdateKnownJunctions()
{
	for (const auto Junction : FNWorldAssemblyRegistry::GetCellJunctionComponents())
	{
		// We want to make sure our relative calculations are updated in this situation
		Junction->OnTransformUpdated(nullptr, EUpdateTransformFlags::None, ETeleportType::None);
	}
}

void FNWorldAssemblyEditorUndo::CompareAgainstSidecar()
{
	for (const auto Root : FNWorldAssemblyRegistry::GetCellRootComponents())
	{
		// Do not process undo on proxy-based actors
		if (ANCellActor* CellActor = Root->GetNCellActor();
			CellActor != nullptr && !CellActor->WasSpawnedFromProxy())
		{
			// bActorDirty is a transient, non-reflected flag recomputed on every undo/redo, so it
			// is intentionally not journaled; Modify() here would capture nothing and must not run.
			CellActor->SetActorDirty(CellActor->HasDifferencesFromSidecar());
		}
	}
}

bool FNWorldAssemblyEditorUndo::MatchesContext(const FTransactionContext& InContext,
                                         const TArray<TPair<UObject*, FTransactionObjectEvent>>& TransactionObjectContexts) const
{
	for (const auto Pair : TransactionObjectContexts)
	{
		// Actor
		if (const AActor* Actor = Cast<AActor>(Pair.Key);
			Actor != nullptr)
		{
			if (FNWorldAssemblyEditorUtils::EffectsGeneratedData(Actor)) return true;
		}

		// Component
		if (const UActorComponent* Component = Cast<UActorComponent>(Pair.Key);
			Component != nullptr)
		{

			if (FNWorldAssemblyEditorUtils::EffectsGeneratedData(Component)) return true;
			if (FNWorldAssemblyEditorUtils::EffectsGeneratedData(Component->GetOwner())) return true;
		}
	}

	return false;
}
