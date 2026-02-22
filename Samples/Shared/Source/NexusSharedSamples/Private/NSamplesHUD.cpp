// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSamplesHUD.h"

#include "Engine/Canvas.h"

void ANSamplesHUD::Tick(float DeltaSeconds)
{

	Super::Tick(DeltaSeconds);
}

void ANSamplesHUD::DrawHUD()
{

	Super::DrawHUD();

	if (bHideHUD) return;
	
	if (Canvas)
	{
		if (Font == nullptr)
		{
			Font = GEngine->GetMediumFont();
		}

		// Calculate background size
		float OutHeight;
		float OutWidth;
		GetTextSize(FString::Printf(TEXT("Screenshot Multiplier (%i): -/="), ScreenshotMultiplier), OutWidth, OutHeight, Font, TextScale);
		float InstructionWidth = OutWidth;
		GetTextSize(CurrentCameraName, OutWidth, OutHeight, Font, TextScale);
		float CalculatedTextWidth = OutWidth;
		if (InstructionWidth > CalculatedTextWidth)
		{
			CalculatedTextWidth = InstructionWidth;
		}
		const float CalculatedTextHeight = OutHeight * 9;

		// Text position
		const FVector2D TextPosition(ScreenSafeZone, ScreenSafeZone);
		
		// Render Background
		FVector2D RectPosition = TextPosition;
		RectPosition.X -= BackgroundPadding;
		RectPosition.Y -= BackgroundPadding;
		DrawRect(BackgroundColor, RectPosition.X, RectPosition.Y,
			CalculatedTextWidth + (BackgroundPadding * 2), CalculatedTextHeight + (BackgroundPadding * 2));

		// Render Text
		const FString DebugText = FString::Printf(
			TEXT("%s\n\n"
				"Next Display: Tab, }\n"
				"Previous Display: {\n"
				"Select Pawn: Backslash\n"
				"Toggle HUD: Backspace\n"
				"Screenshot: F12\n"
				"Screenshot Multiplier (%i): -/=\n"
				"Auto-Screenshot: F10"),
				*CurrentCameraName,
				ScreenshotMultiplier);
		
		DrawText(DebugText, TextColor, TextPosition.X, TextPosition.Y, Font, TextScale);
	}
}
