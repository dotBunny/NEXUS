// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellLevelInstanceLocator.generated.h"

/**
 * Network-friendly identifier for a single ANCellLevelInstance.
 *
 * Carries enough information (ticket + spawn GUID + location) to let a client correlate a replicated
 * reference with a local instance after streaming finishes, without replicating the actor pointer itself.
 */
USTRUCT()
struct NEXUSPROCGEN_API FNCellLevelInstanceLocator
{
	GENERATED_BODY()

	/** Ticket of the owning UNProcGenOperation. */
	UPROPERTY()
	uint32 OperationTicket = 0;

	/** Spawn GUID unique within the owning operation. */
	UPROPERTY()
	FGuid LevelInstanceSpawnGuid;

	/** World location where the level instance should be placed; quantized for network efficiency. */
	UPROPERTY()
	FVector_NetQuantize Location = FVector::ZeroVector;
};