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
		
		DrawRect(BackgroundColor, 0,  Canvas->SizeY - BottomBarHeight, Canvas->SizeX, BottomBarHeight);
		
		const float TopLine = Canvas->SizeY - BottomBarHeight + 10;
		float ColumnSpacing  = 10;
		FVector2D LineCursor;
		
		DrawMonoText("Map", TitleColor, ColumnSpacing, TopLine, LineCursor);
		if (bIsLoading)
		{
			DrawMonoText(FString::Printf(TEXT("%i: Loading Map"), MapIndex), TextColor, ColumnSpacing, LineCursor.Y + LineSpacing, LineCursor);
		}
		else
		{
			DrawMonoText(FString::Printf(TEXT("%i: %s"), MapIndex, *GetWorld()->GetMapName()), TextColor, ColumnSpacing, LineCursor.Y + LineSpacing, LineCursor);
		}
		DrawMonoText(TEXT("Change Map (,) / Next Map (.)"), TextColor, ColumnSpacing, LineCursor.Y + LineSpacing, LineCursor);
		
		NextColumn(ColumnSpacing, LineCursor);
		
		DrawMonoText("Camera", TitleColor, ColumnSpacing, TopLine, LineCursor);
		DrawMonoText(CurrentCameraName, TextColor, ColumnSpacing, LineCursor.Y + LineSpacing, LineCursor);
		DrawMonoText(TEXT("Previous Display (Tab,[) / Next Display (])"), TextColor, ColumnSpacing, LineCursor.Y + LineSpacing, LineCursor);
		DrawMonoText(TEXT("Select Pawn (Backslash)"), TextColor, ColumnSpacing, LineCursor.Y + LineSpacing, LineCursor);
		
		NextColumn(ColumnSpacing, LineCursor);
		
		DrawMonoText("Player", TitleColor, ColumnSpacing, TopLine, LineCursor);		
		DrawMonoText(PlayerPosition.ToString(), TextColor, ColumnSpacing, LineCursor.Y + LineSpacing, LineCursor);
		DrawMonoText(PlayerRotation.ToString(), TextColor, ColumnSpacing, LineCursor.Y + LineSpacing, LineCursor);
		
		NextColumn(ColumnSpacing, LineCursor);
		
		DrawMonoText("Screenshot", TitleColor, ColumnSpacing, TopLine, LineCursor);
		DrawMonoText(FString::Printf(TEXT("Multiplier (x%i): - / ="), ScreenshotMultiplier), TextColor, ColumnSpacing, LineCursor.Y + LineSpacing, LineCursor);
		DrawMonoText(TEXT("Capture (F12)"), TextColor, ColumnSpacing, LineCursor.Y + LineSpacing, LineCursor);
		DrawMonoText(TEXT("Capture Sequence (F10)"), TextColor, ColumnSpacing, LineCursor.Y + LineSpacing, LineCursor);
		
		NextColumn(ColumnSpacing, LineCursor);
		
		DrawMonoText("Extras", TitleColor, ColumnSpacing, TopLine, LineCursor);
		DrawMonoText(TEXT("Toggle HUD (Backspace)"), TextColor, ColumnSpacing, LineCursor.Y + LineSpacing, LineCursor);
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

void ANSamplesHUD::NextColumn(float& ColumnSpacing, FVector2D& LineCursor)
{
	if (LineCursor.X < 300)
	{
		ColumnSpacing += 350;	
	}
	else
	{
		ColumnSpacing += LineCursor.X + 50;	
	}
	LineCursor.X = 0;
	LineCursor.Y = 0;
}
