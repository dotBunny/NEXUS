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

private:
	bool bHideHUD = false;
	int ScreenshotMultiplier = 1;
	FString CurrentCameraName;
	FVector PlayerPosition;
	FRotator PlayerRotation;
	
	UPROPERTY()
	TObjectPtr<UFont> MonospaceFont = nullptr;
	
	UPROPERTY()
	TObjectPtr<UFont> MediumFont = nullptr;
	
	UPROPERTY()
	TObjectPtr<UFont> LargeFont = nullptr;

	const FLinearColor TextColor = FNColor::BlueLight;
	const FLinearColor TitleColor = FLinearColor::White;
	const FLinearColor BackgroundColor = FNColor::HalfBlack;
	const float TextScale = 1.f;
	const float TitleScale = 1.f;
	const float ScreenSafeZone = 50.f;
	const float BackgroundPadding = 20.f;
	
	void DrawMonoText(FString const& Text, FLinearColor Color, float ScreenPositionX, float ScreenPositionY, FVector2D& OutColumnCursor);
};