// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellJunctionBlockerActor.generated.h"


UCLASS(ClassGroup = "NEXUS", DisplayName = "Cell Junction Blocker Actor")
class NEXUSPROCGEN_API ANCellJunctionBlockerActor : public AActor
{
	GENERATED_BODY()

public:
	void Block(FVector Location, FRotator Rotation, FIntVector2 UnitSize);
};

