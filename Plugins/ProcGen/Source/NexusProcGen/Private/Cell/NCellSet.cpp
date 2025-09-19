// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NCellSet.h"
#include "Cell/NCell.h"

void UNCellSet::BuildCellMap(UNCellSet* CellSet, TMap<TObjectPtr<UNCell>, FNCellSetEntry>& OutCellMap, TArray<UNCellSet*>& OutProcessedSets)
{
	// Base level entries
	for (FNCellSetEntry Entry : CellSet->Entries)
	{
		if (!OutCellMap.Find(Entry.Cell.Get()))
		{
			OutCellMap.Add(Entry.Cell.Get(), Entry);
		}
		else
		{
			// TODO: what do we want to do if settings are different? like uniqueness? or weighting?
		}
	}

	// Additional Set Recursion
	for (UNCellSet* ChildCellSet : CellSet->AdditionalSets)
	{
		if (OutProcessedSets.Contains(CellSet)) continue;

		// Add before recursion to stop double
		OutProcessedSets.Add(CellSet);
		
		BuildCellMap(ChildCellSet, OutCellMap, OutProcessedSets);
	}
}
