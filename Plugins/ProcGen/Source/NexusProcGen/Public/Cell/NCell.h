// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCellJunctionDetails.h"
#include "NCellRootDetails.h"
//#include "PCGDataAsset.h"
#include "NCell.generated.h"

/**
 * The data-only representation of a NCellActor and its details.
 * @note The UNCell is effectively a side-car file to NCellActor.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "Cell")
class NEXUSPROCGEN_API UNCell : public UDataAsset
{
	friend class FNProcGenEditorUtils;
	
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, DisplayName="Level")
	TSoftObjectPtr<UWorld> World;

	UPROPERTY(VisibleAnywhere)
	FNCellRootDetails Root;

	UPROPERTY(VisibleAnywhere)
	TMap<int32, FNCellJunctionDetails> Junctions;

	/**
	 * TODO: Explicit data structure populated to work with PCG
	 */
	//UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "PCG", meta = (NoResetToDefault))
	//FPCGDataCollection DataCollection;

private:

	UPROPERTY(VisibleAnywhere)
	int Version = 0;
};