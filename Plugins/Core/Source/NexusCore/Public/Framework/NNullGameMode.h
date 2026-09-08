// Copyright dotBunny Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "NNullGameMode.generated.h"

/**
 *  DETHOL Bootstrap GameMode
 */
UCLASS(DisplayName="NEXUS | Null GameMode")
class NEXUSCORE_API ANNullGameMode : public AGameMode
{
	GENERATED_BODY()

	ANNullGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
