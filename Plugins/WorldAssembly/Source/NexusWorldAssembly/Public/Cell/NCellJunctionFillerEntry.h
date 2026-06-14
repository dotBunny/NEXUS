// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "GameplayTagContainer.h"
#include "Collections/NGameplayTagCounterConstraint.h"

#include "NCellJunctionFillerEntry.generated.h"


USTRUCT(BlueprintType)
struct NEXUSWORLDASSEMBLY_API FNCellJunctionFillerEntry
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, DisplayName="Actor", Category="Object", meta=(MustImplement="/Script/NexusWorldAssembly.NCellJunctionFiller"))
	TSubclassOf<AActor> Actor;
	
	UPROPERTY(EditAnywhere, DisplayName="Offset", Category="Object")
	FTransform Offset = FTransform::Identity;
	
	/** Tags required to be found in Context Tags for allowance to place this cell. **/
	UPROPERTY(EditAnywhere, DisplayName="Required Context Tags", Category="Tagging", meta = (ToolTip="Tags required to be found in Context Tags for allowance to place this cell."))
	FGameplayTagContainer RequiredContextTags;
	
	UPROPERTY(EditAnywhere, DisplayName="Tag Counter Constraints", Category="Tagging", meta = (ToolTip="TagCounter constraints that must pass for this cell to be included in cell selection. If a tag is constrained but not present in the current Tag Counter, a value of 0 is compared against."))
	TArray<FNGameplayTagCounterConstraint> TagCounterConstraints;
	
	UPROPERTY(EditAnywhere, DisplayName="Tag Counter Operations", Category="Tagging", meta = (ToolTip="Operations to apply to the TagCounter if the cell is placed/used."))
	TArray<FNGameplayTagCounterOperation> TagCounterOperations;
	
	/** 
	 * Relative weight for random selection during generation.
	 * @note Higher values increase the probability of this cell being chosen.
	 */
	UPROPERTY(EditAnywhere)
	int32 Weighting = 1;
};
