// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellLevelInstanceLocator.generated.h"

USTRUCT()
struct NEXUSPROCGEN_API FNCellLevelInstanceLocator
{
	GENERATED_BODY()

	UPROPERTY()
	uint32 OperationTicket = 0;

	UPROPERTY()
	FGuid LevelInstanceSpawnGuid;

	UPROPERTY()
	FVector_NetQuantize Location = FVector::ZeroVector;
};