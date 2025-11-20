// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NTissue.h"
#include "Cell/NCell.h"

void UNTissue::BuildTissueMap(UNTissue* Tissue, TMap<TObjectPtr<UNCell>, FNTissueEntry>& OutCellMap, TArray<UNTissue*>& OutProcessedSets)
{
	// Base level entries
	for (FNTissueEntry Entry : Tissue->Cells)
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
	for (UNTissue* Additional : Tissue->AdditionalTissue)
	{
		if (OutProcessedSets.Contains(Tissue)) continue;

		// Add before recursion to stop double
		OutProcessedSets.Add(Tissue);
		
		BuildTissueMap(Additional, OutCellMap, OutProcessedSets);
	}
}
