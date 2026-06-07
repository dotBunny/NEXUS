// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Collections/NGameplayTagCounter.h"
#include "NCellAssemblyData.generated.h"

/**
 * Per-cell assembly metadata recorded on a generated cell, identifying the operation and graph node that produced it.
 */
USTRUCT(BlueprintType)
struct FNCellAssemblyData
{
	GENERATED_BODY()

	/** Ticket of the operation that generated this cell. */
	UPROPERTY(VisibleInstanceOnly)
	int32 OperationTicket = 0;

	/** Identifier of the assembly graph node this cell was generated from. */
	UPROPERTY(VisibleInstanceOnly)
	int32 NodeIdentifier = 0;

	/** Seed used to generate this cell. */
	UPROPERTY(VisibleInstanceOnly)
	uint64 Seed = 0;

	/** Assembly tags applied to this cell during the assembly operation. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	FGameplayTagContainer AssemblyTags;

	/** The final ContextTags for the Assembly Operation. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	FGameplayTagContainer ContextTags;
	
	/** The state of the ContextTags when this cell was added. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	FGameplayTagContainer ContextTagsState;
	
	/** The ContextTags this cell added. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	FGameplayTagContainer ContextTagsAdded;
	
	/** The final TagCounter */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	TArray<FNGameplayTagCount> TagCounter;
	
	/** The state of the tag counter when this cell was added. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	TArray<FNGameplayTagCount> TagCounterState;
};
