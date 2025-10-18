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
	Glyphs.Reserve(42);
	
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
	Glyphs.Add(TArray {
		FVector2D(0,0),
		FVector2D(0,4),

		FVector2D(0,2),
		FVector2D(2,2),

		FVector2D(2,2),
		FVector2D(3,3),

		FVector2D(3,3),
		FVector2D(3,4),
		
		FVector2D(2,2),
		FVector2D(3,1),

		FVector2D(3,1),
		FVector2D(3,0)
	});

	// L
	Glyphs.Add(TArray {
		FVector2D(0,4),
		FVector2D(0,0),

		FVector2D(0,0),
		FVector2D(3,0)
	});

	// M
	Glyphs.Add(TArray {
		FVector2D(0,0),
		FVector2D(0,4),

		FVector2D(0,4),
		FVector2D(1.5f,2),

		FVector2D(1.5f,2),
		FVector2D(3,4),

		FVector2D(3,4),
		FVector2D(3,0)
	});
	
	// N
	Glyphs.Add(TArray {
		FVector2D(0,0),
		FVector2D(0,4),

		FVector2D(0,4),
		FVector2D(3,0),

		FVector2D(3,0),
		FVector2D(3,4)
	});

	// O
	Glyphs.Add(TArray {
		FVector2D(0,0),
		FVector2D(0, 4),
	
		FVector2D(0, 4),
		FVector2D(3, 4),

		FVector2D(3, 4),
		FVector2D(3, 0),

		FVector2D(3, 0),
		FVector2D(0, 0)
	});

	// P
	Glyphs.Add(TArray {
		FVector2D(0,0),
		FVector2D(0, 4),

		FVector2D(0, 4),
		FVector2D(3, 4),

		FVector2D(3, 4),
		FVector2D(3, 2),

		FVector2D(3, 2),
		FVector2D(0, 2)
	});

	// Q
	Glyphs.Add(TArray {
		FVector2D(0,0),
		FVector2D(0, 4),
	
		FVector2D(0, 4),
		FVector2D(3, 4),

		FVector2D(3, 4),
		FVector2D(3, 0),

		FVector2D(3, 0),
		FVector2D(0, 0),

		FVector2D(3, 0),
		FVector2D(2, 1)
	});

	// R
	Glyphs.Add(TArray {
		FVector2D(0,0),
		FVector2D(0, 4),

		FVector2D(0, 4),
		FVector2D(3, 4),

		FVector2D(3, 4),
		FVector2D(3, 2),

		FVector2D(3, 2),
		FVector2D(0, 2),

		FVector2D(0, 2),
		FVector2D(3, 0)
	});

	// S
	Glyphs.Add(TArray {
		FVector2D(3,4),
		FVector2D(0,4),

		FVector2D(0,4),
		FVector2D(0,2),

		FVector2D(0,2),
		FVector2D(3,2),

		FVector2D(3,2),
		FVector2D(3,0),

		FVector2D(3,0),
		FVector2D(0,0)
	});

	// T
	Glyphs.Add(TArray {
		FVector2D(0,4),
		FVector2D(3,4),

		FVector2D(1.5f,4),
		FVector2D(1.5f,0)
	});

	// U
	Glyphs.Add(TArray {
		FVector2D(0,4),
		FVector2D(0,0),

		FVector2D(0,0),
		FVector2D(3,0),

		FVector2D(3,0),
		FVector2D(3,4)
	});

	// V
	Glyphs.Add(TArray {
		FVector2D(0,4),
		FVector2D(1.5f,0),

		FVector2D(1.5f,0),
		FVector2D(3,4)
	});

	// W
	Glyphs.Add(TArray {
		FVector2D(0,4),
		FVector2D(0,0),

		FVector2D(0,0),
		FVector2D(1.5f,1.5f),

		FVector2D(1.5f,1.5f),
		FVector2D(3,0),

		FVector2D(3,0),
		FVector2D(3,4)
	});

	// X
	Glyphs.Add(TArray {
		FVector2D(0,4),
		FVector2D(3,0),

		FVector2D(0,0),
		FVector2D(3,4)
	});

	// Y
	Glyphs.Add(TArray {
		FVector2D(1.5f,0),
		FVector2D(1.5f,2),

		FVector2D(1.5f,2),
		FVector2D(0,4),

		FVector2D(1.5f,2),
		FVector2D(3,4)
	});

	// Z
	Glyphs.Add(TArray {
		FVector2D(0,4),
		FVector2D(3,4),

		FVector2D(3,4),
		FVector2D(0,0),

		FVector2D(0,0),
		FVector2D(3,0)
	});

	
	// -
	Glyphs.Add(TArray {
		FVector2D(1,2),
		FVector2D(2,2)
	});

	// [
	Glyphs.Add(TArray {
		FVector2D(2,4),
		FVector2D(0,4),

		FVector2D(0,4),
		FVector2D(0,0),

		FVector2D(0,0),
		FVector2D(2,0)
	});

	// ]
	Glyphs.Add(TArray {
		FVector2D(1,4),
		FVector2D(3,4),

		FVector2D(3,4),
		FVector2D(3,0),

		FVector2D(3,0),
		FVector2D(1,0)
	});

	// (
	Glyphs.Add(TArray {
		FVector2D(2,4),
		FVector2D(1,4),

		FVector2D(1,4),
		FVector2D(0,3),

		FVector2D(0,3),
		FVector2D(0,1),

		FVector2D(0,1),
		FVector2D(1,0),

		FVector2D(1,0),
		FVector2D(2,0)
	});

	// )
	Glyphs.Add(TArray {
		FVector2D(1,4),
		FVector2D(2,4),

		FVector2D(2,4),
		FVector2D(3,3),

		FVector2D(3,3),
		FVector2D(3,1),

		FVector2D(3,1),
		FVector2D(2,0),

		FVector2D(2,0),
		FVector2D(1,0)
	});
}

int FNPrimitiveDrawingUtils::CharToGlyphIndex(const char Character)
{
	switch (Character)
	{
	case ' ':
	case '\n':
	case '\r':
	case '\t':
		return -1;
	case '0':
		return 1;
	case '1':
		return 2;
	case '2':
		return 3;
	case '3':
		return 4;
	case '4':
		return 5;
	case '5':
		return 6;
	case '6':
		return 7;
	case '7':
		return 8;
	case '8':
		return 9;
	case '9':
		return 10;	
	
	case 'a':
	case 'A':
		return 11;
	case 'b':
	case 'B':
		return 12;
	case 'c':
	case 'C':
		return 13;
	case 'd':
	case 'D':
		return 14;
	case 'e':
	case 'E':
		return 15;
	case 'f':
	case 'F':
		return 16;
	case 'g':
	case 'G':
		return 17;
	case 'h':
	case 'H':
		return 18;
	case 'i':
	case 'I':
		return 19;
	case 'j':
	case 'J':
		return 20;
	case 'k':
	case 'K':
		return 21;
	case 'l':
	case 'L':
		return 22;
	case 'm':
	case 'M':
		return 23;	
	case 'n':
	case 'N':
		return 24;	
	case 'o':
	case 'O':
		return 25;	
	case 'p':
	case 'P':
		return 26;	
	case 'q':
	case 'Q':
		return 27;	
	case 'r':
	case 'R':
		return 28;	
	case 's':
	case 'S':
		return 29;	
	case 't':
	case 'T':
		return 30;	
	case 'u':
	case 'U':
		return 31;	
	case 'v':
	case 'V':
		return 32;	
	case 'w':
	case 'W':
		return 33;	
	case 'x':
	case 'X':
		return 34;	
	case 'y':
	case 'Y':
		return 35;	
	case 'z':
	case 'Z':
		return 36;

	case '-':
		return 37;
	case '[':
		return 38;
	case ']':
		return 39;
	case '(':
		return 40;
	case ')':
		return 41;		
		
	default:
		return 0;
	}
}
