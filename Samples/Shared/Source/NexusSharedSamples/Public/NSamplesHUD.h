// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NColor.h"
#include "GameFramework/HUD.h"
#include "NSamplesHUD.generated.h"

/**
 * The HUD used in the NEXUS samples.
 */
UCLASS()
class NEXUSSHAREDSAMPLES_API ANSamplesHUD : public AHUD
{
	GENERATED_BODY()
public:
	
	virtual void Tick(float DeltaSeconds) override;

	virtual void DrawHUD() override;
	
	void ToggleVisibility() { bHideHUD = !bHideHUD;}
	void SetVisibility(const bool bVisible) { bHideHUD = !bVisible; }
	void SetScreenshotMultiplier(const int Multiplier) { ScreenshotMultiplier = Multiplier; };
	void SetCurrentCameraName(FString Name) { CurrentCameraName = Name; };
	void SetPlayerPosition(const FVector& UpdatePosition) { PlayerPosition = UpdatePosition; };
	void SetPlayerRotation(const FRotator& UpdateRotation) { PlayerRotation = UpdateRotation; };
	void SetMapIndex(const int Index) { MapIndex = Index; };
	void SetMapChangeFlag() { bIsLoading = true; };


private:
	bool bHideHUD = false;
	bool bIsLoading = false;
	int ScreenshotMultiplier = 1;
	int MapIndex = 0;
	FString CurrentCameraName;
	FVector PlayerPosition;
	FRotator PlayerRotation;
	
	float CachedBackgroundWidth = 0.f;
	
	UPROPERTY()
	TObjectPtr<UFont> MonospaceFont = nullptr;
	
	void DrawMonoText(const FString& Text, const float TextScale, FLinearColor Color, float Indent, FVector2D& OutCursor);
};