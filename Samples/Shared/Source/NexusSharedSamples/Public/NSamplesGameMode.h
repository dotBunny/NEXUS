// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NSamplesGameMode.generated.h"


/**
 * A GameMode used in the NEXUS samples.
 */
UCLASS(Config = Game)
class NEXUSSHAREDSAMPLES_API ANSamplesGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	explicit ANSamplesGameMode(const FObjectInitializer& ObjectInitializer);
};