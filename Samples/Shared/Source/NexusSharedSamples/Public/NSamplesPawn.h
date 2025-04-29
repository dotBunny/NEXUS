// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "NSamplesPawn.generated.h"

class UWidgetInteractionComponent;

/**
 * A pawn used in the NEXUS samples that has a few extra bits on it.
 */
UCLASS(Config = Game)
class NEXUSSHAREDSAMPLES_API ANSamplesPawn : public ADefaultPawn
{
	GENERATED_BODY()

	ANSamplesPawn(const FObjectInitializer& ObjectInitializer);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NEXUS")
	TObjectPtr<UWidgetInteractionComponent> WidgetInteraction;
};