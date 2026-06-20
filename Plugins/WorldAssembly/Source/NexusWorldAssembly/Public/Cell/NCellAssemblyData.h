// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Collections/NGameplayTagCounter.h"
#include "NCellJunctionDetails.h"
#include "NCellLinkDetails.h"
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

	/** Is this cell on the shortest-path hot path (spokes from start)? */
	UPROPERTY(VisibleInstanceOnly)
	bool bHotPathShortest = false;

	/** Is this cell on the sequential hot path (nearest-first visiting chain)? */
	UPROPERTY(VisibleInstanceOnly)
	bool bHotPathSequential = false;

	/** Assembly tags applied to this cell during the assembly operation. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	FGameplayTagContainer AssemblyTags;

	/** The final ContextTags for the Assembly Operation. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	FGameplayTagContainer ContextTags;

	/** The ContextTags this cell added. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	FGameplayTagContainer ContextTagsAdded;

	/** The final TagCounter */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	TArray<FNGameplayTagCount> TagCounter;

	/** World-space details for every junction on this cell, captured from the source graph node. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	TArray<FNCellJunctionDetails> JunctionDetails;

	/** Per-junction connection state (what each junction linked to, if anything) generated alongside JunctionDetails. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
	TArray<FNCellLinkDetails> LinkDetails;
};
