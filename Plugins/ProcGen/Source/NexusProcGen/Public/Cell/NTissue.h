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
	
	UPROPERTY(EditAnywhere)
	bool bCanBeStartNode = true;
	
	UPROPERTY(EditAnywhere)
	bool bCanBeEndNode = true;
	
	UPROPERTY(EditAnywhere)
	int MinimumCount = -1;
	
	UPROPERTY(EditAnywhere)
	int MaximumCount = -1;
	
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