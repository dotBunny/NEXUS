// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Cell/NTissue.h"
#include "Cell/NCell.h"

void UNTissue::BuildTissueMap(UNTissue* Tissue, TMap<TObjectPtr<UNCell>, FNTissueEntry>& OutCellMap, FNTissueTagGroups& OutTagGroups, TArray<UNTissue*>& OutProcessedSets)
{
	OutTagGroups.EnsureBaseTags();
	OutTagGroups.AppendTags(Tissue->TagGroups);

	// Base level entries
	for (FNTissueEntry Entry : Tissue->Cells)
	{
		TObjectPtr<UNCell> Cell = Entry.Cell.LoadSynchronous();
		if (Cell == nullptr) continue;
		FNTissueEntry* FoundEntry = OutCellMap.Find(Cell);
		if (!FoundEntry)
		{
			OutCellMap.Add(Cell, Entry);
		}
		else
		{
			// We are going to combine the tags when we have an existing entry
			FoundEntry->AssemblyTags.AppendTags(Entry.AssemblyTags);
		}
	}


	// Additional Set Recursion
	for (auto Additional : Tissue->AdditionalTissue)
	{
		TObjectPtr<UNTissue> AdditionalTissue = Additional.LoadSynchronous();
		if (AdditionalTissue == nullptr || OutProcessedSets.Contains(AdditionalTissue)) continue;

		// Add before recursion to stop double
		OutProcessedSets.Add(AdditionalTissue);

		BuildTissueMap(AdditionalTissue, OutCellMap, OutTagGroups, OutProcessedSets);
	}
}
