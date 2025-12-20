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
	 * Can only be used once during the generation process from this set.
	 */
	UPROPERTY(EditAnywhere)
	bool bIsUnique = false;

	UPROPERTY(EditAnywhere)
	int Weighting = 1;
	
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UNCell> Cell;
};

/**
 * A collection of NCells
 * @remark Can think of this as a definition of multiple cell types used with tissue.
 */
UCLASS(DisplayName = "NEXUS: Tissue")
class NEXUSPROCGEN_API UNTissue : public UDataAsset
{
	GENERATED_BODY()

public:
	
	static void BuildTissueMap(UNTissue* Tissue, TMap<TObjectPtr<UNCell>, FNTissueEntry>& OutCellMap, TArray<UNTissue*>& OutProcessedSets);
	
	UPROPERTY(EditAnywhere, meta=(TitleProperty="{Cell}"))
	TArray<FNTissueEntry> Cells;

	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UNTissue>> AdditionalTissue;
};