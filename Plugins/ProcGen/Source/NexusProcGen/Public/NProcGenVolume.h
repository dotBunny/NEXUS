// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameFramework/Volume.h"

#include "NProcGenVolume.generated.h"

class UNProcGenComponent;

UCLASS(BlueprintType, DisplayName = "NProcGen Volume")
class NEXUSPROCGEN_API ANProcGenVolume : public AVolume
{
	GENERATED_BODY()

public:
	ANProcGenVolume(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Nexus")
	TObjectPtr<UNProcGenComponent> NProcGenComponent;
};
