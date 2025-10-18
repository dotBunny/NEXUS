// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NPrimitiveDrawingUtils.h"

TArray<TArray<FVector2D>> FNPrimitiveDrawingUtils::Glyphs = TArray<TArray<FVector2D>>();


void FNPrimitiveDrawingUtils::DrawString(FPrimitiveDrawInterface* PDI, FString& String, FVector Position,
	FRotator Rotation, FLinearColor ForegroundColor, float Scale)
{
	// Ensure our glyphs are created
	if (Glyphs.Num() == 0) GenerateGlyphs();
	
	
	//PDI->AddReserveLines()
}

void FNPrimitiveDrawingUtils::GenerateGlyphs()
{
	// A glyph is based off a 3(w)x4(h) bottom-left oriented grid.
	// It should consist of paired points which will be used to draw the corresponding lines.

	Glyphs.Empty();
	Glyphs.Reserve(25);
	
	// Undefined
	Glyphs.Add(TArray {
		FVector2D(0,0),
		FVector2D(0, 4),
		
		FVector2D(0, 4),
		FVector2D(3, 4),

		FVector2D(3, 4),
		FVector2D(3, 0),

		FVector2D(3, 0),
		FVector2D(0, 0),

		FVector2D(0, 4),
		FVector2D(3, 0),

		FVector2D(3, 4),
		FVector2D(0, 0),
	});

	// 0
	Glyphs.Add(TArray {
		FVector2D(0,0),
		FVector2D(0, 4),
		
		FVector2D(0, 4),
		FVector2D(3, 4),

		FVector2D(3, 4),
		FVector2D(3, 0),

		FVector2D(3, 0),
		FVector2D(0, 0),

		FVector2D(3, 4),
		FVector2D(0, 0)
	});

	// 1
	Glyphs.Add(TArray {
		FVector2D(0,0),
		FVector2D(3,0),
		
		FVector2D(1.5f,0),
		FVector2D(1.5f,4),

		FVector2D(1.5f,4),
		FVector2D(0,3)
	});

	// 2
	Glyphs.Add(TArray {
		FVector2D(0,3),
		FVector2D(0,4),

		FVector2D(0,4),
		FVector2D(3,4),

		FVector2D(3,4),
		FVector2D(3,3),

		FVector2D(3,3),
		FVector2D(0,0),

		FVector2D(0,0),
		FVector2D(3,0),
	});

	// 3
	Glyphs.Add(TArray {
		FVector2D(0,4),
		FVector2D(3,4),

		FVector2D(3,4),
		FVector2D(3,0),

		FVector2D(3,0),
		FVector2D(0,0),

		FVector2D(1,2),
		FVector2D(3,2),
	});

	// 4
	Glyphs.Add(TArray{
		FVector2D(3,0),
		FVector2D(3,4),

		FVector2D(0,4),
		FVector2D(0,2),

		FVector2D(0,2),
		FVector2D(3,2),
	});

	// 5
	Glyphs.Add(TArray {
		FVector2D(0,1),
		FVector2D(0,0),

		FVector2D(0,0),
		FVector2D(3,0),

		FVector2D(3,0),
		FVector2D(3,2),

		FVector2D(3,2),
		FVector2D(0,2),

		FVector2D(0,2),
		FVector2D(0,4),
		
		FVector2D(0,4),
		FVector2D(3,4)
	});

	// 6
	Glyphs.Add(TArray {
		FVector2D(3,4),
		FVector2D(0,4),

		FVector2D(0,4),
		FVector2D(0,0),

		FVector2D(0,0),
		FVector2D(3,0),

		FVector2D(3,0),
		FVector2D(3,2),

		FVector2D(3,2),
		FVector2D(0,2)
	});

	// 7
	Glyphs.Add(TArray {
		FVector2D(0,4),
		FVector2D(3,4),

		FVector2D(3,4),
		FVector2D(0,0)
	});

	// 8
	Glyphs.Add(TArray {
		FVector2D(0,0),
		FVector2D(0, 4),

		FVector2D(0, 4),
		FVector2D(3, 4),

		FVector2D(3, 4),
		FVector2D(3, 0),

		FVector2D(3, 0),
		FVector2D(0, 0),

		FVector2D(0, 2),
		FVector2D(3, 2)
	});

	// 9
	Glyphs.Add(TArray {
		FVector2D(0,1),
		FVector2D(0,0),

		FVector2D(0,0),
		FVector2D(3,0),

		FVector2D(3,0),
		FVector2D(3,4),

		FVector2D(3,4),
		FVector2D(0,4),

		FVector2D(0,4),
		FVector2D(0,2),

		FVector2D(0,2),
		FVector2D(3,2)
	});
	
	// A
	Glyphs.Add(TArray {
		FVector2D(0,0),
		FVector2D(0, 4),
		
		FVector2D(0, 4),
		FVector2D(3, 4),

		FVector2D(3, 4),
		FVector2D(3, 0),

		FVector2D(0, 2),
		FVector2D(3, 2)
	});
	
	// B
	Glyphs.Add(TArray {
		FVector2D(3,4),
		FVector2D(0,4),

		FVector2D(0,4),
		FVector2D(0,0),

		FVector2D(0,0),
		FVector2D(3,0),

		FVector2D(0,2),
		FVector2D(2,2),

		FVector2D(2,2),
		FVector2D(3,3),

		FVector2D(2,2),
		FVector2D(3,1)
	});

	// C
	Glyphs.Add(TArray {
		FVector2D(3,4),
		FVector2D(0,4),

		FVector2D(0,4),
		FVector2D(0,0),

		FVector2D(0,0),
		FVector2D(3,0)
	});

	// D
	Glyphs.Add(TArray {
		FVector2D(2,4),
		FVector2D(0,4),

		FVector2D(0,4),
		FVector2D(0,0),

		FVector2D(0,0),
		FVector2D(2,0),

		FVector2D(2,4),
		FVector2D(3,3),

		FVector2D(3,3),
		FVector2D(3,1),

		FVector2D(3,1),
		FVector2D(2,0)
	});

	// E
	Glyphs.Add(TArray {
		FVector2D(3,4),
		FVector2D(0,4),

		FVector2D(0,4),
		FVector2D(0,0),

		FVector2D(0,0),
		FVector2D(3,0),

		FVector2D(0,2),
		FVector2D(2,2)
	});

	// F
	Glyphs.Add(TArray {
		FVector2D(3,4),
		FVector2D(0,4),

		FVector2D(0,4),
		FVector2D(0,0),

		FVector2D(0,2),
		FVector2D(2,2)
	});

	// G
	Glyphs.Add(TArray {
		FVector2D(3,4),
		FVector2D(0,4),

		FVector2D(0,4),
		FVector2D(0,0),

		FVector2D(0,0),
		FVector2D(3,0),

		FVector2D(3,0),
		FVector2D(3,2),

		FVector2D(3,2),
		FVector2D(2,2)
	});

	// H
	Glyphs.Add(TArray {
		FVector2D(0,4),
		FVector2D(0,0),
		
		FVector2D(3,4),
		FVector2D(3,0),

		FVector2D(0,2),
		FVector2D(3,2)
	});

	// I
	Glyphs.Add(TArray {
		FVector2D(0,4),
		FVector2D(3,4),

		FVector2D(1.5f, 4),
		FVector2D(1.5f, 0),
		
		FVector2D(0,0),
		FVector2D(3,0),
	});

	// J
	Glyphs.Add(TArray {
		FVector2D(0,4),
		FVector2D(3,4),

		FVector2D(3,4),
		FVector2D(3,1),

		FVector2D(3,1),
		FVector2D(2,0),

		FVector2D(2,0),
		FVector2D(1,0),

		FVector2D(1,0),
		FVector2D(0,1),
	});

	// K
	Glyphs.Add(TArray<FVector2D>{});

	// L
	Glyphs.Add(TArray<FVector2D>{});

	// M
	Glyphs.Add(TArray<FVector2D>{});
	
	// N
	Glyphs.Add(TArray<FVector2D>{});

	// O
	Glyphs.Add(TArray<FVector2D>{});

	// P
	Glyphs.Add(TArray<FVector2D>{});

	// Q
	Glyphs.Add(TArray<FVector2D>{});

	// R
	Glyphs.Add(TArray<FVector2D>{});

	// S
	Glyphs.Add(TArray<FVector2D>{});

	// T
	Glyphs.Add(TArray<FVector2D>{});

	// U
	Glyphs.Add(TArray<FVector2D>{});

	// V
	Glyphs.Add(TArray<FVector2D>{});

	// W
	Glyphs.Add(TArray<FVector2D>{});

	// X
	Glyphs.Add(TArray<FVector2D>{});

	// Y
	Glyphs.Add(TArray<FVector2D>{});

	// Z
	Glyphs.Add(TArray<FVector2D>{});

	// -
	Glyphs.Add(TArray<FVector2D>{});

	// :
	Glyphs.Add(TArray<FVector2D>{});

	// [
	Glyphs.Add(TArray<FVector2D>{});

	// ]
	Glyphs.Add(TArray<FVector2D>{});

	// (
	Glyphs.Add(TArray<FVector2D>{});

	// )
	Glyphs.Add(TArray<FVector2D>{});
	
}
