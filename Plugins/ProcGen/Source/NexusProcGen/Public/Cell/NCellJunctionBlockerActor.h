// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCellJunctionBlockerActor.generated.h"


/**
 * Actor placed at a junction when ENCellJunctionRequirements::AllowBlocking applies and the junction
 * has no outgoing connection — physically caps the socket so players can't traverse through an unfilled junction.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Cell Junction Blocker Actor")
class NEXUSPROCGEN_API ANCellJunctionBlockerActor : public AActor
{
	GENERATED_BODY()

public:
	/**
	 * Position and size the blocker to cap the specified junction socket.
	 * @param Location World location of the junction.
	 * @param Rotation World rotation of the junction.
	 * @param UnitSize Junction socket size in grid units (width, height).
	 */
	void Block(FVector Location, FRotator Rotation, FIntVector2 UnitSize);
};

