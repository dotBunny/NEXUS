// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NTissue.h"
#include "Cell/NCell.h"

void UNTissue::BuildTissueMap(UNTissue* Tissue, TMap<TObjectPtr<UNCell>, FNTissueEntry>& OutCellMap, TArray<UNTissue*>& OutProcessedSets)
{
	// Base level entries
	
	for (FNTissueEntry Entry : Tissue->Cells)
	{
		TObjectPtr<UNCell> Cell = Entry.Cell.LoadSynchronous();
		if (!OutCellMap.Find(Cell))
		{
			OutCellMap.Add(Cell, Entry);
		}
		else
		{
			// TODO: what do we want to do if settings are different? like uniqueness? or weighting?
		}
	}

	// Additional Set Recursion
	for (auto Additional : Tissue->AdditionalTissue)
	{
		TObjectPtr<UNTissue> AdditionalTissue = Additional.LoadSynchronous();
		if (OutProcessedSets.Contains(AdditionalTissue)) continue;

		// Add before recursion to stop double
		OutProcessedSets.Add(AdditionalTissue);
		
		BuildTissueMap(AdditionalTissue, OutCellMap, OutProcessedSets);
	}
}
