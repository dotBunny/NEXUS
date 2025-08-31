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
		// Example: Draw text to the screen
		FString CameraName = TEXT("BP_NSamples_Test");
		FVector2D TextPosition(Canvas->SizeX * 0.5f, Canvas->SizeY * 0.1f); // Center top
		FLinearColor TextColor = FLinearColor::White;
		UFont* HudFont = GEngine->GetSmallFont(); // Get a default font

		// Adjust position for text centering
		float Width;
		float Height;
		GetTextSize(CameraName, Width, Height, HudFont);
		TextPosition.X -= Width * 0.5f;

		DrawText(CameraName, TextColor, TextPosition.X, TextPosition.Y, HudFont);
	}


	/* TODO: Keys on screen
	 * Next Camera: Tab / Right Bracket
	 * Previous Camera: Left Bracket
	 * Toggle HUD: Backslash
	 * Screenshot: F12
	 *
	 * -/+ to increase multiplier
	 */
}
