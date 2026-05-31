// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCanvasUtils.h"

#include "CanvasItem.h"
#include "CanvasTypes.h"

void FNCanvasUtils::DrawCanvasTextBox(FNMultiLineTextBoxCanvasItem* TextBox, FCanvas* Canvas, FVector2D TopLeftPosition)
{
	// Ensure processed
	if (TextBox->IsDirty())
	{
		TextBox->Process(10.f);
	}
	
	// Check we're still good
	if (!TextBox->HasContent())
	{
		return;
	}
	
	FVector2D Padding(10.0f, 10.0f); // X and Y internal padding
	FVector2D BoxSize(
		TextBox->Width + (Padding.X * 2.5f),
		TextBox->Height + (Padding.Y * 2.0f)
	);
		
	// Draw Background
	FCanvasTileItem BackgroundTile(TopLeftPosition, BoxSize, TextBox->BackgroundColor);
	BackgroundTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BackgroundTile);
		
	// Draw Border
	FCanvasBoxItem BorderItem(TopLeftPosition, BoxSize);
	BorderItem.LineThickness = 1.0f;
	BorderItem.SetColor(TextBox->BorderColor);
	Canvas->DrawItem(BorderItem);
		
	for (int32 i = 0; i < TextBox->Lines.Num(); ++i)
	{
		FVector2D TextPosition = TopLeftPosition + FVector2D(Padding.X, TextBox->LinePositions[i]);
		if (TextBox->bIsLargeFont[i])
		{
			FCanvasTextItem TextItem(TextPosition, TextBox->Lines[i], GEngine->GetLargeFont(), TextBox->Colors[i]);
			TextItem.EnableShadow(FLinearColor::Black);
			Canvas->DrawItem(TextItem);
    
		}
		else
		{
			FCanvasTextItem TextItem(TextPosition, TextBox->Lines[i], GEngine->GetSmallFont(), TextBox->Colors[i]);
			TextItem.EnableShadow(FLinearColor::Black);
			Canvas->DrawItem(TextItem);
		}
	}
	
}
