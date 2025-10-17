// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NPrimitiveDrawingUtils.h"

TArray<TArray<FVector2D>> FNPrimitiveDrawingUtils::Glyphs = TArray<TArray<FVector2D>>();


void FNPrimitiveDrawingUtils::DrawString(FPrimitiveDrawInterface* PDI, FString& String, FVector Position,
	FRotator Rotation, FLinearColor ForegroundColor, float Scale, bool bBackground, FLinearColor BackgroundColor)
{
	// Ensure our glyphs are created
	if (Glyphs.Num() == 0) GenerateGlyphs();
	
	
	//PDI->AddReserveLines()
}

void FNPrimitiveDrawingUtils::GenerateGlyphs()
{
	Glyphs.Empty();
	
	Glyphs.Reserve(24);
	
	// Undefined (0)
	// have to be line pairs!
	// BOX? with X in it ? 0,0, 
	Glyphs.Add(TArray<FVector2D>{
		FVector2D(0,0),
		FVector2D(0, 1),
		FVector2D(1,1),
		FVector2D(1,0),
		FVector2D(0,0),
		
		
		FVector2D(1,0),
		
		
		FVector2D(0,0),
		FVector2D(1,1),
	});
	
	// A
	
}
