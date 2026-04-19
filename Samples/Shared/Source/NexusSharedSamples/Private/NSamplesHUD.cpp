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
	
	// Don't show if no one wants it or is around
	if (bHideHUD || FApp::IsUnattended()) return;
	
	if (Canvas)
	{
		DrawRect(FNColor::HalfBlack, 0,  0, CachedBackgroundWidth, Canvas->SizeY);
		FVector2D LineCursor =  FVector2D(CachedBackgroundWidth, 20);

		constexpr FLinearColor TextColor = FNColor::BlueLight;
		const FLinearColor TitleColor = FLinearColor::White;
		
		DrawMonoText("Map", 1.1f, TitleColor, 15, LineCursor);
		if (bIsLoading)
		{
			DrawMonoText(FString::Printf(TEXT("%i: Loading Map"), MapIndex), 1.0f, TextColor, 20, LineCursor);
		}
		else
		{
			DrawMonoText(FString::Printf(TEXT("%i: %s"), MapIndex, *GetWorld()->GetMapName()), 1.0f, TextColor, 20, LineCursor);
		}
		DrawMonoText(TEXT("Change Map (,) / Next Map (.)"), 1.0f, TextColor, 20, LineCursor);
		
		LineCursor.Y += 20;
		
		DrawMonoText("Player", 1.1f,  TitleColor, 15, LineCursor);		
		DrawMonoText(PlayerPosition.ToString(),1.0f, TextColor, 20, LineCursor);
		DrawMonoText(PlayerRotation.ToString(),1.0f, TextColor, 20, LineCursor);
		
		LineCursor.Y += 20;
		
		DrawMonoText("Camera", 1.1f,  TitleColor, 15, LineCursor);
		DrawMonoText(CurrentCameraName, 1.0f, TextColor, 20, LineCursor);
		DrawMonoText(TEXT("Previous Display (Tab,[) / Next Display (])"), 1.0f, TextColor, 20, LineCursor);
		DrawMonoText(TEXT("Select Pawn (Backslash)"), 1.0f, TextColor, 20, LineCursor);
		
		LineCursor.Y += 20;
		
		DrawMonoText("Screenshot", 1.1f, TitleColor, 15, LineCursor);
		DrawMonoText(FString::Printf(TEXT("Multiplier (x%i): - / ="), ScreenshotMultiplier), 1.0f, TextColor, 20, LineCursor);
		DrawMonoText(TEXT("Capture (F12)"), 1.0f, TextColor, 20, LineCursor);
		DrawMonoText(TEXT("Capture Sequence (F10)"), 1.0f, TextColor, 20, LineCursor);
		

		// Footer
		LineCursor.Y = Canvas->SizeY - 40;
		DrawMonoText(TEXT("Toggle (Backspace)"), 1.1f, FLinearColor::Yellow, 20, LineCursor);
		
		CachedBackgroundWidth = LineCursor.X;
	}
}

void ANSamplesHUD::DrawMonoText(const FString& Text, const float TextScale, const FLinearColor Color, const float Indent, FVector2D& OutCursor)
{
	if (MonospaceFont == nullptr)
	{
		MonospaceFont = GEngine->GetMonospaceFont();
	}

	float OutWidth, OutHeight;
	GetTextSize(Text, OutWidth, OutHeight, MonospaceFont, TextScale);
	DrawText(Text, Color, Indent, OutCursor.Y, MonospaceFont, TextScale);

	const float WidthWithPadding = OutWidth + 40;
	if (WidthWithPadding > OutCursor.X) { OutCursor.X = WidthWithPadding; }
	OutCursor.Y = OutCursor.Y + OutHeight + 2; // Line Spacing
};
