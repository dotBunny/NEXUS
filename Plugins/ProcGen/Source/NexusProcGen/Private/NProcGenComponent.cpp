// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenComponent.h"

#include "NCoreMinimal.h"
#include "Cell/NCell.h"
#include "Cell/NCellSet.h"

FString UNProcGenComponent::GetDebugLabel()
{
	return GetOwner()->GetActorNameOrLabel();
}

TMap<TObjectPtr<UNCell>, FNCellSetEntry> UNProcGenComponent::GetCellMap() const
{
	TMap<TObjectPtr<UNCell>, FNCellSetEntry> CellMap;
	TArray<UNCellSet*> ReferencedSets;

	UNCellSet::BuildCellMap(CellSet, CellMap, ReferencedSets);
	for (const auto& Set : ReferencedSets)
	{
		N_LOG(Log, TEXT("[UNProcGenComponent::BuildCellMap] %s References UNCellSet: %s"), *this->GetName(), *Set->GetName())
	}
	return MoveTemp(CellMap);
}
