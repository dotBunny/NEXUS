// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "NSamplesPawn.generated.h"

class UWidgetInteractionComponent;
class ANSamplesDisplayActor;

/**
 * A pawn used in the NEXUS samples that has a few extra bits on it.
 */
UCLASS(DisplayName = "NEXUS: Samples Pawn", Config = Game)
class NEXUSSHAREDSAMPLES_API ANSamplesPawn : public ADefaultPawn
{
	GENERATED_BODY()

	ANSamplesPawn(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:

	void OnNextDisplay();
	void OnPreviousDisplay();
	void OnToggleHUD();
	void OnScreenshot();
	void OnReturnToPawn();
	void OnResolutionIncrease();
	void OnResolutionDecrease();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NEXUS")
	TObjectPtr<UWidgetInteractionComponent> WidgetInteraction;
private:

	void ChangeView(ANSamplesDisplayActor* DisplayActor);
	int CameraIndex = -1;
	int ResolutionMultiplier = 1;
};