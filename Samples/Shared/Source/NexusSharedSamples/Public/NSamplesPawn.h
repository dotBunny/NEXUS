// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "NSamplesPawn.generated.h"

class UWidgetInteractionComponent;
class ANSamplesDisplayActor;


enum class ENSamplesScreenshotState : uint8
{
	NotRunning = 0,
	Prepare = 1,
	SelectView = 2,
	RequestScreenshot = 3,
	Cleanup = 4
};

/**
 * A pawn used in the NEXUS samples that has a few extra bits on it.
 */
UCLASS(ClassGroup="NEXUS", DisplayName = "Samples Pawn", Config = Game, NotPlaceable)
class NEXUSSHAREDSAMPLES_API ANSamplesPawn : public ADefaultPawn
{
	GENERATED_BODY()

	ANSamplesPawn(const FObjectInitializer& ObjectInitializer);

	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

protected:

	void OnNextDisplay();
	void OnPreviousDisplay();
	void OnToggleHUD();
	void OnScreenshot();
	void OnReturnToPawn();
	void OnResolutionIncrease();
	void OnResolutionDecrease();
	void OnAutoScreenshot();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NEXUS")
	TObjectPtr<UWidgetInteractionComponent> WidgetInteraction;
private:

	void ChangeView(ANSamplesDisplayActor* DisplayActor);
	void CheckScreenshotState();
	int CameraIndex = -1;
	int ResolutionMultiplier = 1;
	int FrameSkipDefault = 5;
	int FrameSkipCounter = FrameSkipDefault;
	ENSamplesScreenshotState ScreenshotState;
	
};