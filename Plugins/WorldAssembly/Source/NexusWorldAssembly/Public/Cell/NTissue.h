// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameplayTagContainer.h"
#include "NTissueEntry.h"
#include "NTissueTagGroups.h"
#include "Collections/NGameplayTagCounter.h"
#include "Collections/NGameplayTagCounterConstraint.h"
#include "Engine/DataAsset.h"
#include "Types/NCardinalRotation.h"
#include "NTissue.generated.h"

class UNCell;

/**
 * A collection of NCells
 * @remark Can think of this as a definition of multiple cell types used with tissue.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Tissue",
	meta = (DocsURL="https://nexus-framework.com/docs/plugins/world-assembly/types/tissue"))
class NEXUSWORLDASSEMBLY_API UNTissue : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Flatten a tissue (and any referenced AdditionalTissue) into a single cell-to-entry map.
	 * @param Tissue The root tissue to start from.
	 * @param OutCellMap Populated with every cell reachable from Tissue and the effective entry to use.
	 * @param OutTagGroups Accumulated tag groups from all visited tissues.
	 * @param OutProcessedSets Tracks tissue assets already visited so cycles do not cause infinite recursion.
	 */
	static void BuildTissueMap(UNTissue* Tissue, TMap<TObjectPtr<UNCell>, FNTissueEntry>& OutCellMap, 
		FNTissueTagGroups& OutTagGroups, 
		TArray<UNTissue*>& OutProcessedSets);
	
	/** Tag groups that drive placement behavior for this tissue's cells. */
	UPROPERTY(EditAnywhere, DisplayName="Assembly Tag Groups")
	FNTissueTagGroups TagGroups = FNTissueTagGroups();
	
	/** The cells that directly belong to this tissue, along with per-cell generation constraints. */
	UPROPERTY(EditAnywhere, meta=(TitleProperty="{Cell}"))
	TArray<FNTissueEntry> Cells;

	/** Additional tissue assets merged into this one during BuildTissueMap. */
	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<UNTissue>> AdditionalTissue;
};