// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCanvasUtils.h"

#include "CanvasItem.h"
#include "CanvasTypes.h"

void FNCanvasUtils::DrawCanvasTextBox(FNMultiLineTextBoxCanvasItem* TextBox, FCanvas* Canvas, FVector2D TopLeftPosition)
{
	if (TextBox == nullptr || Canvas == nullptr)
	{
		return;
	}

	// Ensure processed
	if (TextBox->IsDirty())
	{
		TextBox->Process(10);
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
		
	for (const auto& Line : TextBox->Lines)
	{
		const FVector2D TextPosition = TopLeftPosition + FVector2D(Padding.X, Line.Position);
		UFont* Font = Line.bIsLargeFont ? GEngine->GetLargeFont() : GEngine->GetSmallFont();

		FCanvasTextItem TextItem(TextPosition, Line.Text, Font, Line.Color);
		TextItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(TextItem);
	}
}
