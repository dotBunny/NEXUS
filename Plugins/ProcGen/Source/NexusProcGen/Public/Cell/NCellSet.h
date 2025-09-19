// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Engine/DataAsset.h"
#include "NCellSet.generated.h"

class UNCell;

USTRUCT()
struct NEXUSPROCGEN_API FNCellSetEntry
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
 * A collection of NCellNodes
 */
UCLASS()
class NEXUSPROCGEN_API UNCellSet : public UDataAsset
{
	GENERATED_BODY()

public:
	
	static void BuildCellMap(UNCellSet* CellSet, TMap<TObjectPtr<UNCell>, FNCellSetEntry>& OutCellMap, TArray<UNCellSet*>& OutProcessedSets);
	
	UPROPERTY(EditAnywhere, meta=(TitleProperty="{Cell}"))
	TArray<FNCellSetEntry> Entries;

	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UNCellSet>> AdditionalSets;
};