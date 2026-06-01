// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NCellAssemblyData.generated.h"

USTRUCT(BlueprintType)
struct FNCellAssemblyData
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleInstanceOnly)
	uint32 OperationTicket = 0;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer OutputTags;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer ContextTags;
};