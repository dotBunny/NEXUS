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
		constexpr float BottomBarHeight = 100.f;
		constexpr float LineSpacing = 2;
		constexpr float ColumnSpacing = 50;
		
		DrawRect(BackgroundColor, 0,  Canvas->SizeY - BottomBarHeight, Canvas->SizeX, BottomBarHeight);
		
		const float BottomTextTop = Canvas->SizeY - BottomBarHeight + 10;
		float BottomLeftSpacing  = 10;
		
		FVector2D LineCursor;
		
		DrawMonoText("Map", TitleColor, BottomLeftSpacing, BottomTextTop, LineCursor);
		DrawMonoText(GetWorld()->GetMapName(), TextColor, BottomLeftSpacing, LineCursor.Y + LineSpacing, LineCursor);
		DrawMonoText(TEXT("Previous Map (,) / Next Map (.)"), TextColor, BottomLeftSpacing, LineCursor.Y + LineSpacing, LineCursor);
		
		BottomLeftSpacing += LineCursor.X + ColumnSpacing;
		LineCursor.X = 0;
		LineCursor.Y = 0;
		
		DrawMonoText("Camera", TitleColor, BottomLeftSpacing, BottomTextTop, LineCursor);
		DrawMonoText(CurrentCameraName, TextColor, BottomLeftSpacing, LineCursor.Y + LineSpacing, LineCursor);
		DrawMonoText(TEXT("Previous Display (Tab,[) / Next Display (])"), TextColor, BottomLeftSpacing, LineCursor.Y + LineSpacing, LineCursor);
		DrawMonoText(TEXT("Select Pawn (Backslash)"), TextColor, BottomLeftSpacing, LineCursor.Y + LineSpacing, LineCursor);
		
		BottomLeftSpacing += LineCursor.X + ColumnSpacing;
		LineCursor.X = 0;
		LineCursor.Y = 0;
		
		DrawMonoText("Screenshot", TitleColor, BottomLeftSpacing, BottomTextTop, LineCursor);
		DrawMonoText(FString::Printf(TEXT("Multiplier (%i): - / ="), ScreenshotMultiplier), TextColor, BottomLeftSpacing, LineCursor.Y + LineSpacing, LineCursor);
		DrawMonoText(TEXT("Capture (F12)"), TextColor, BottomLeftSpacing, LineCursor.Y + LineSpacing, LineCursor);
		DrawMonoText(TEXT("Capture Sequence (F10)"), TextColor, BottomLeftSpacing, LineCursor.Y + LineSpacing, LineCursor);
		
		BottomLeftSpacing += LineCursor.X + ColumnSpacing;
		LineCursor.X = 0;
		LineCursor.Y = 0;
		
		DrawMonoText("Extras", TitleColor, BottomLeftSpacing, BottomTextTop, LineCursor);
		DrawMonoText(TEXT("Toggle HUD (Backspace)"), TextColor, BottomLeftSpacing, LineCursor.Y + LineSpacing, LineCursor);
		
		BottomLeftSpacing += LineCursor.X + ColumnSpacing;
		LineCursor.X = 0;
		LineCursor.Y = 0;
		
		DrawMonoText("Player", TitleColor, BottomLeftSpacing, BottomTextTop, LineCursor);		
		DrawMonoText(PlayerPosition.ToString(), TextColor, BottomLeftSpacing, LineCursor.Y + LineSpacing, LineCursor);
		DrawMonoText(PlayerRotation.ToString(), TextColor, BottomLeftSpacing, LineCursor.Y + LineSpacing, LineCursor);
	}
}

void ANSamplesHUD::DrawMonoText(FString const& Text, const FLinearColor Color, const float ScreenPositionX,
	const float ScreenPositionY, FVector2D& OutColumnCursor)
{
	if (MonospaceFont == nullptr)
	{
		MonospaceFont = GEngine->GetMonospaceFont();
	}

	float OutWidth, OutHeight;
	GetTextSize(Text, OutWidth, OutHeight, MonospaceFont, TextScale);
	DrawText(Text, Color, ScreenPositionX, ScreenPositionY, MonospaceFont, TextScale);
	
	if (OutWidth > OutColumnCursor.X) { OutColumnCursor.X = OutWidth; }
	OutColumnCursor.Y = ScreenPositionY+OutHeight;
}
