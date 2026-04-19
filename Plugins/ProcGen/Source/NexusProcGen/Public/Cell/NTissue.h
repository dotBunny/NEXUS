// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Engine/DataAsset.h"
#include "NTissue.generated.h"

class UNCell;

USTRUCT()
struct NEXUSPROCGEN_API FNTissueEntry
{
	GENERATED_BODY()
	
	/**
	 * Whether the NCellLevelInstance should be spawned always relevant for networking purposes.
	 */	
	UPROPERTY(EditAnywhere)
	bool bAlwaysRelevant = false;
	
	/** 
	 * Whether this cell can be used as a starting node in the generated FNProcGenGraph. 
	 */	
	UPROPERTY(EditAnywhere)
	bool bCanBeStartNode = true;
	
	/** 
	 * Whether this cell can be used as an ending/terminal node in the generated FNProcGenGraph.
	 */	
	UPROPERTY(EditAnywhere)
	bool bCanBeEndNode = true;
	
	/** 
	 * A minimum number of times this cell must be used in the generated FNProcGenGraph.
	 * @note A value of -1 indicates no minimum constraint.
	 */	
	UPROPERTY(EditAnywhere)
	int MinimumCount = -1;
	
	/** 
	 * The maximum number of times this cell can be used in the generated FNProcGenGraph.
	 * @note A value of -1 indicates no maximum constraint.
	 */	
	UPROPERTY(EditAnywhere)
	int MaximumCount = -1;
	
	/**
	 * The minimum number of cell links away this cell must be to be used again.
	 * @note A value of 0 indicates no constraint.
	 */
	UPROPERTY(EditAnywhere,meta=(ClampMin=0, UIMin=0))
	int MinimumNodeDistance = 1;
	
	/** 
	 * Relative weight for random selection during generation.
	 * @note Higher values increase the probability of this cell being chosen.
	 */
	UPROPERTY(EditAnywhere)
	int Weighting = 1;
	
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UNCell> Cell;
};

/**
 * A collection of NCells
 * @remark Can think of this as a definition of multiple cell types used with tissue.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Tissue")
class NEXUSPROCGEN_API UNTissue : public UDataAsset
{
	GENERATED_BODY()

public:
	
	static void BuildTissueMap(UNTissue* Tissue, TMap<TObjectPtr<UNCell>, FNTissueEntry>& OutCellMap, TArray<UNTissue*>& OutProcessedSets);
	
	UPROPERTY(EditAnywhere, meta=(TitleProperty="{Cell}"))
	TArray<FNTissueEntry> Cells;

	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<UNTissue>> AdditionalTissue;
};