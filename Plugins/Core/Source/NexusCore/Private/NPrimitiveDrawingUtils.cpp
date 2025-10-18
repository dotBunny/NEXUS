// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NPrimitiveDrawingUtils.h"

TArray<TArray<FVector2D>> FNPrimitiveDrawingUtils::Glyphs = TArray<TArray<FVector2D>>();


void FNPrimitiveDrawingUtils::DrawString(FPrimitiveDrawInterface* PDI, FString& String, const FVector& Position,
	const FRotator& Rotation, const FLinearColor ForegroundColor, const float Scale)
{
	
	// TODO: Add orientation / left / right / center  based?
	
	// Ensure our glyphs are created
	if (Glyphs.Num() == 0) GenerateGlyphs();
	
	const float WorkingScale = Scale * 25;


	// Reserve some room, assuming an average of 5 lines per character
	TArray<TCHAR>& Characters = String.GetCharArray();
	PDI->AddReserveLines(SDPG_World, 5 * Characters.Num());
	FVector CurrentPosition = Position;
	int LineIndex = 0;
	const float LineHeight = ((4 + 2) * WorkingScale) * -1;
	const FVector CharacterPostOffset = FVector(-4 * WorkingScale, 0.f, 0.f);
	const FVector CharacterOffset = FVector(-3 * WorkingScale, 0.f, 0.f);
	
	for (const auto Character : Characters)
	{
		switch (const int GlyphIndex = CharToGlyphIndex(Character))
		{
		case -4: // String terminator, stop now
			return;
		case -3: // Tab (4-spaces)
			CurrentPosition = Position + (CharacterOffset * 4);
			break;
		case -2: // New Line
			LineIndex++;
			CurrentPosition = Position + FVector(0.f, LineHeight * LineIndex, 0.f);
			break;
		case -1: // Space
			CurrentPosition += CharacterOffset;
			break;
		default:
			TArray<FVector2D>& Points = Glyphs[GlyphIndex];
			const int PointCount = Points.Num();
			for (int i = 0; i < PointCount; i += 2)
			{
				// Scale our points and bring them into 3D
				FVector StartPoint = CurrentPosition + FVector((Points[i].X * -1) * WorkingScale, Points[i].Y * WorkingScale, 0.f);
				FVector EndPoint = CurrentPosition + FVector((Points[i + 1].X * -1) * WorkingScale, Points[i + 1].Y * WorkingScale, 0.f);
			
				
				// Rotate points around base origin w/ rotation
				StartPoint = Position + Rotation.RotateVector(StartPoint - Position);
				EndPoint = Position + Rotation.RotateVector(EndPoint - Position);
				
				// Rotate points with offset in position
				PDI->DrawLine(StartPoint, EndPoint, ForegroundColor, SDPG_World, 2.f);
			}
			CurrentPosition += CharacterPostOffset;
			break;
		}
	}
}

void FNPrimitiveDrawingUtils::GenerateGlyphs()
{
	// A glyph is based off a 3(w)x4(h) bottom-left oriented grid.
	// It should consist of paired points which will be used to draw the corresponding lines.

	Glyphs.Empty();
	Glyphs.Reserve(50);
	
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
		
		FVector2D(3,3),
		FVector2D(3,4),

		FVector2D(2,2),
		FVector2D(3,1),
		
		FVector2D(3,1),
		FVector2D(3,0)
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
	
	// _
	Glyphs.Add(TArray {
		FVector2D(0,0),
		FVector2D(3,0)
	});
	
	// : TODO
	Glyphs.Add(TArray { 
		FVector2D(0,0),
		FVector2D(3,0)
	});
	
	// @ TODO
	Glyphs.Add(TArray { 
		FVector2D(0,0),
		FVector2D(3,0)
	});
	
	// ! TODO
	Glyphs.Add(TArray { 
		FVector2D(0,0),
		FVector2D(3,0)
	});
	
	// # TODO
	Glyphs.Add(TArray { 
		FVector2D(0,0),
		FVector2D(3,0)
	});
	
	// + TODO
	Glyphs.Add(TArray { 
		FVector2D(0,0),
		FVector2D(3,0)
	});
	
	// & TODO
	Glyphs.Add(TArray { 
		FVector2D(0,0),
		FVector2D(3,0)
	});
	
	// * TODO
	Glyphs.Add(TArray { 
		FVector2D(0,0),
		FVector2D(3,0)
	});
}

int FNPrimitiveDrawingUtils::CharToGlyphIndex(const TCHAR Character)
{
	switch (Character)
	{
	case '\0':
		return -4;
	case '\t':
		return -3;
	case '\r':
	case '\n':
		return -2;
	case ' ':
		return -1;
		
	// Numbers
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
	
	// Alpha
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

	// Additional Characters
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
	case '_':
		return 42;
	case ':':
		return 43;
	case '@':
		return 44;
		
	case '!':
		return 45;
	case '#':
		return 46;
	case '+':
		return 47;	
	case '&':
		return 48;
	case '*':
		return 49;	
		
	default:
		return 0;
	}
}