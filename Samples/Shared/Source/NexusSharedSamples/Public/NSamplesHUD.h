// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
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
	void SetScreenshotMultiplier(const int Multiplier) { ScreenshotMultiplier = Multiplier; };

private:
	bool bHideHUD = false;
	int ScreenshotMultiplier = 1;
};