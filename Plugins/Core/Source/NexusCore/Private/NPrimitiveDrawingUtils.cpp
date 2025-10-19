// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NPrimitiveDrawingUtils.h"
#include "Developer/NPrimitiveDrawStringSettings.h"

TArray<TArray<FNPrimitiveDrawStringPoint>> FNPrimitiveDrawingUtils::Glyphs = TArray<TArray<FNPrimitiveDrawStringPoint>>();
bool FNPrimitiveDrawingUtils::bHasGeneratedGlyphs = false;

void FNPrimitiveDrawingUtils::DrawString(FPrimitiveDrawInterface* PDI, FString& String, const FVector& Position,
	const FRotator& Rotation, const FLinearColor ForegroundColor, const float Scale, const ENPrimitiveDrawStringAlignment Alignment,
	const ENPrimitiveDrawStringPivot Pivot, const float LineHeight, const float Thickness)
{
	// Ensure our glyphs are created
	if (!bHasGeneratedGlyphs) GenerateGlyphs();

	// Setup the working scale to multiply our glyph points by
	const float WorkingScale = Scale * 12; // Our pseudo font-size

	// Reserve some room, assuming an average of 5 lines per character
	TArray<TCHAR>& Characters = String.GetCharArray();

	PDI->AddReserveLines(SDPG_World, 6 * Characters.Num(), false, true);

	FVector CurrentPosition = Position;
	int LineIndex = 0;
	const float WorkingLineHeight = ((8 + LineHeight) * WorkingScale) * -1;
	const FVector CharacterPostOffset = FVector(-8 * WorkingScale, 0.f, 0.f);
	const FVector CharacterOffset = FVector(-6 * WorkingScale, 0.f, 0.f);
	const UE::Math::TRotationMatrix<double> RotationMatrix = UE::Math::TRotationMatrix(Rotation);
	
	for (const auto Character : Characters)
	{
		switch (const int GlyphIndex = Character)
		{
		case 0: // String terminator, stop now
			return;
		case 9: // Tab (4-spaces)
			CurrentPosition = CurrentPosition + (CharacterPostOffset * 4);
			break;
		case 10: // New Line
		case 13:
			LineIndex++;
			CurrentPosition = Position + FVector(0.f, WorkingLineHeight * LineIndex, 0.f);
			break;
		case 32: // Space
			CurrentPosition += CharacterOffset;
			break;
		default:

			// Get reference to glyph data
			TArray<FNPrimitiveDrawStringPoint>& Points = Glyphs[GlyphIndex];
			
			// Check our point count, if we don't have any at this point, it is considered a bad character
			int PointCount = Points.Num();
			if (PointCount  == 0)
			{
				Points = Glyphs[0];
				PointCount = 12;
			}

			// Draw our glyph
			for (int i = 0; i < PointCount; i += 2)
			{
				// Scale our points and bring them into 3D
				FVector StartPoint = CurrentPosition + FVector(Points[i].X * WorkingScale, Points[i].Y * WorkingScale, 0.f);
				FVector EndPoint = CurrentPosition + FVector(Points[i + 1].X * WorkingScale, Points[i + 1].Y * WorkingScale, 0.f);
				
				// Rotate points around base origin w/ rotation
				StartPoint = Position + RotationMatrix.TransformVector(StartPoint - Position);
				EndPoint = Position + RotationMatrix.TransformVector(EndPoint - Position);
				
				// Rotate points with offset in position
				PDI->DrawLine(StartPoint, EndPoint, ForegroundColor, SDPG_World, Thickness);
			}

			// Offset the position to get ready for the next character
			CurrentPosition += CharacterPostOffset;
			break;
		}
	}
}


void FNPrimitiveDrawingUtils::GenerateGlyphs()
{
	if (bHasGeneratedGlyphs) return;
	
	// A glyph is based off a 6(w)x8(h) bottom-left oriented grid.
	// It should consist of paired points which will be used to draw the corresponding lines.
	Glyphs.AddZeroed(126);
	
	// Replace null with unknown-character as we'll use it.
	Glyphs[0] = TArray {
		FNPrimitiveDrawStringPoint(0, 0),
		FNPrimitiveDrawStringPoint(0, 8),
		
		FNPrimitiveDrawStringPoint(0, 8),
		FNPrimitiveDrawStringPoint(-6, 8),

		FNPrimitiveDrawStringPoint(-6, 8),
		FNPrimitiveDrawStringPoint(-6, 0),

		FNPrimitiveDrawStringPoint(-6, 0),
		FNPrimitiveDrawStringPoint(0, 0),

		FNPrimitiveDrawStringPoint(0, 8),
		FNPrimitiveDrawStringPoint(-6, 0),

		FNPrimitiveDrawStringPoint(-6, 8),
		FNPrimitiveDrawStringPoint(0, 0),
	};
	Glyphs.Shrink();

	// #
	Glyphs[35] = TArray {
		FNPrimitiveDrawStringPoint(-2,7),
		FNPrimitiveDrawStringPoint(-2,1),

		FNPrimitiveDrawStringPoint(-4,7),
		FNPrimitiveDrawStringPoint(-4,1),
		
		FNPrimitiveDrawStringPoint(-1,3),
		FNPrimitiveDrawStringPoint(-5,3),

		FNPrimitiveDrawStringPoint(-1,5),
		FNPrimitiveDrawStringPoint(-5,5)
	};

	// '
	Glyphs[39] = TArray {
		FNPrimitiveDrawStringPoint(-2,8),
		FNPrimitiveDrawStringPoint(-2,6)
	};
	
	// (
	Glyphs[40] = TArray {
		FNPrimitiveDrawStringPoint(-4,8),
		FNPrimitiveDrawStringPoint(-2,8),

		FNPrimitiveDrawStringPoint(-2,8),
		FNPrimitiveDrawStringPoint(0,6),

		FNPrimitiveDrawStringPoint(0,6),
		FNPrimitiveDrawStringPoint(0,2),

		FNPrimitiveDrawStringPoint(0,2),
		FNPrimitiveDrawStringPoint(-2,0),

		FNPrimitiveDrawStringPoint(-2,0),
		FNPrimitiveDrawStringPoint(-4,0)
	};

	// )
	Glyphs[41] = TArray {
		FNPrimitiveDrawStringPoint(-2,8),
		FNPrimitiveDrawStringPoint(-4,8),

		FNPrimitiveDrawStringPoint(-4,8),
		FNPrimitiveDrawStringPoint(-6,6),

		FNPrimitiveDrawStringPoint(-6,6),
		FNPrimitiveDrawStringPoint(-6,2),

		FNPrimitiveDrawStringPoint(-6,2),
		FNPrimitiveDrawStringPoint(-4,0),

		FNPrimitiveDrawStringPoint(-4, 0),
		FNPrimitiveDrawStringPoint(-2,0)
	};
	
	// +
	Glyphs[43] = TArray { 
		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(-6,4),
		
		FNPrimitiveDrawStringPoint(-3,7),
		FNPrimitiveDrawStringPoint(-3,1)
	};

	// '
	Glyphs[44] = TArray {
		FNPrimitiveDrawStringPoint(-1,1),
		FNPrimitiveDrawStringPoint(-1,0),

		FNPrimitiveDrawStringPoint(-1,0),
		FNPrimitiveDrawStringPoint(0,-1)
	};
		
	// -
	Glyphs[45] = TArray {
		FNPrimitiveDrawStringPoint(-2,4),
		FNPrimitiveDrawStringPoint(-4,4)
	};

	// .
	Glyphs[46] = TArray {
		FNPrimitiveDrawStringPoint(0,1),
		FNPrimitiveDrawStringPoint(-1,1),

		FNPrimitiveDrawStringPoint(-1,1),
		FNPrimitiveDrawStringPoint(-1,0),

		FNPrimitiveDrawStringPoint(-1,0),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0,1)
	};

	// Forward slash
	Glyphs[47] = TArray {
		FNPrimitiveDrawStringPoint(-1,0),
		FNPrimitiveDrawStringPoint(-7,8),
	};
	
	// 0
	Glyphs[48] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0, 8),
		
		FNPrimitiveDrawStringPoint(0, 8),
		FNPrimitiveDrawStringPoint(-6, 8),

		FNPrimitiveDrawStringPoint(-6, 8),
		FNPrimitiveDrawStringPoint(-6, 0),

		FNPrimitiveDrawStringPoint(-6, 0),
		FNPrimitiveDrawStringPoint(0, 0),

		FNPrimitiveDrawStringPoint(-6, 8),
		FNPrimitiveDrawStringPoint(0, 0)
	};

	// 1
	Glyphs[49] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-6,0),
		
		FNPrimitiveDrawStringPoint(-3,0),
		FNPrimitiveDrawStringPoint(-3,8),

		FNPrimitiveDrawStringPoint(-3,8),
		FNPrimitiveDrawStringPoint(0,6)
	};

	// 2
	Glyphs[50] = TArray {
		FNPrimitiveDrawStringPoint(0,6),
		FNPrimitiveDrawStringPoint(0,8),

		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(-6,8),

		FNPrimitiveDrawStringPoint(-6,8),
		FNPrimitiveDrawStringPoint(-6,6),

		FNPrimitiveDrawStringPoint(-6,6),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-6,0),
	};

	// 3
	Glyphs[51] = TArray {
		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(-6,8),

		FNPrimitiveDrawStringPoint(-6,8),
		FNPrimitiveDrawStringPoint(-6,0),

		FNPrimitiveDrawStringPoint(-6,0),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(-2,4),
		FNPrimitiveDrawStringPoint(-6,4),
	};

	// 4
	Glyphs[52] = TArray{
		FNPrimitiveDrawStringPoint(-6,0),
		FNPrimitiveDrawStringPoint(-6,8),

		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(0,4),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(-6,4),
	};

	// 5
	Glyphs[53] = TArray {
		FNPrimitiveDrawStringPoint(0,2),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-6,0),

		FNPrimitiveDrawStringPoint(-6,0),
		FNPrimitiveDrawStringPoint(-6,4),

		FNPrimitiveDrawStringPoint(-6,4),
		FNPrimitiveDrawStringPoint(0,4),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(0,8),
		
		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(-6,8)
	};

	// 6
	Glyphs[54] = TArray {
		FNPrimitiveDrawStringPoint(-6,8),
		FNPrimitiveDrawStringPoint(0,8),

		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-6,0),

		FNPrimitiveDrawStringPoint(-6,0),
		FNPrimitiveDrawStringPoint(-6,4),

		FNPrimitiveDrawStringPoint(-6,4),
		FNPrimitiveDrawStringPoint(0,4)
	};

	// 7
	Glyphs[55] = TArray {
		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(-6,8),

		FNPrimitiveDrawStringPoint(-6,8),
		FNPrimitiveDrawStringPoint(0,0)
	};

	// 8
	Glyphs[56] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0, 8),

		FNPrimitiveDrawStringPoint(0, 8),
		FNPrimitiveDrawStringPoint(-6, 8),

		FNPrimitiveDrawStringPoint(-6, 8),
		FNPrimitiveDrawStringPoint(-6, 0),

		FNPrimitiveDrawStringPoint(-6, 0),
		FNPrimitiveDrawStringPoint(0, 0),

		FNPrimitiveDrawStringPoint(0, 4),
		FNPrimitiveDrawStringPoint(-6, 4)
	};

	// 9
	Glyphs[57] = TArray {
		FNPrimitiveDrawStringPoint(0,2),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-6,0),

		FNPrimitiveDrawStringPoint(-6,0),
		FNPrimitiveDrawStringPoint(-6,8),

		FNPrimitiveDrawStringPoint(-6,8),
		FNPrimitiveDrawStringPoint(0,8),

		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(0,4),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(-6,4)
	};

	// :
	Glyphs[58] = TArray {
		FNPrimitiveDrawStringPoint(-2, 1),
		FNPrimitiveDrawStringPoint(-4, 1),

		FNPrimitiveDrawStringPoint(-4, 1),
		FNPrimitiveDrawStringPoint(-4, 3),

		FNPrimitiveDrawStringPoint(-4, 3),
		FNPrimitiveDrawStringPoint(-2, 3),

		FNPrimitiveDrawStringPoint(-2, 3),
		FNPrimitiveDrawStringPoint(-2, 1),

		FNPrimitiveDrawStringPoint(-2, 5),
		FNPrimitiveDrawStringPoint(-4, 5),

		FNPrimitiveDrawStringPoint(-4, 5),
		FNPrimitiveDrawStringPoint(-4, 7),

		FNPrimitiveDrawStringPoint(-4, 7),
		FNPrimitiveDrawStringPoint(-2, 7),

		FNPrimitiveDrawStringPoint(-2, 7),
		FNPrimitiveDrawStringPoint(-2, 5),
	};
	
	// @
	Glyphs[64] = TArray { 
		FNPrimitiveDrawStringPoint(-6,0),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0,8),

		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(-6,8),

		FNPrimitiveDrawStringPoint(-6,8),
		FNPrimitiveDrawStringPoint(-6,2),

		FNPrimitiveDrawStringPoint(-6,2),
		FNPrimitiveDrawStringPoint(-2,2),

		FNPrimitiveDrawStringPoint(-2,2),
		FNPrimitiveDrawStringPoint(-2,6),

		FNPrimitiveDrawStringPoint(-2,6),
		FNPrimitiveDrawStringPoint(-4,6)
	};
	
	// A
	Glyphs[65] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0, 8),
		
		FNPrimitiveDrawStringPoint(0, 8),
		FNPrimitiveDrawStringPoint(-6, 8),

		FNPrimitiveDrawStringPoint(-6, 8),
		FNPrimitiveDrawStringPoint(-6, 0),

		FNPrimitiveDrawStringPoint(0, 4),
		FNPrimitiveDrawStringPoint(-6, 4)
	};
	
	// B
	Glyphs[66] = TArray {
		FNPrimitiveDrawStringPoint(-6,8),
		FNPrimitiveDrawStringPoint(0,8),

		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-6,0),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(-4,4),

		FNPrimitiveDrawStringPoint(-4,4),
		FNPrimitiveDrawStringPoint(-6,6),
		
		FNPrimitiveDrawStringPoint(-6,6),
		FNPrimitiveDrawStringPoint(-6,8),

		FNPrimitiveDrawStringPoint(-4,4),
		FNPrimitiveDrawStringPoint(-6,2),
		
		FNPrimitiveDrawStringPoint(-6,2),
		FNPrimitiveDrawStringPoint(-6,0)
	};

	// C
	Glyphs[67] = TArray {
		FNPrimitiveDrawStringPoint(-6,8),
		FNPrimitiveDrawStringPoint(0,8),

		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-6,0)
	};

	// D
	Glyphs[68] = TArray {
		FNPrimitiveDrawStringPoint(-4,8),
		FNPrimitiveDrawStringPoint(0,8),

		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-4,0),

		FNPrimitiveDrawStringPoint(-4,8),
		FNPrimitiveDrawStringPoint(-6,6),

		FNPrimitiveDrawStringPoint(-6,6),
		FNPrimitiveDrawStringPoint(-6,2),

		FNPrimitiveDrawStringPoint(-6,2),
		FNPrimitiveDrawStringPoint(-4,0)
	};

	// E
	Glyphs[69] = TArray {
		FNPrimitiveDrawStringPoint(-6,8),
		FNPrimitiveDrawStringPoint(0,8),

		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-6,0),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(-4,4)
	};

	// F
	Glyphs[70] = TArray {
		FNPrimitiveDrawStringPoint(-6,8),
		FNPrimitiveDrawStringPoint(0,8),

		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(-4,4)
	};

	// G
	Glyphs[71] = TArray {
		FNPrimitiveDrawStringPoint(-6,8),
		FNPrimitiveDrawStringPoint(0,8),

		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-6,0),

		FNPrimitiveDrawStringPoint(-6,0),
		FNPrimitiveDrawStringPoint(-6,4),

		FNPrimitiveDrawStringPoint(-6,4),
		FNPrimitiveDrawStringPoint(-4,4)
	};

	// H
	Glyphs[72] = TArray {
		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(0,0),
		
		FNPrimitiveDrawStringPoint(-6,8),
		FNPrimitiveDrawStringPoint(-6,0),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(-6,4)
	};

	// I
	Glyphs[73] = TArray {
		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(-6,8),

		FNPrimitiveDrawStringPoint(-3, 8),
		FNPrimitiveDrawStringPoint(-3, 0),
		
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-6,0),
	};

	// J
	Glyphs[74] = TArray {
		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(-6,8),

		FNPrimitiveDrawStringPoint(-6,8),
		FNPrimitiveDrawStringPoint(-6,2),

		FNPrimitiveDrawStringPoint(-6,2),
		FNPrimitiveDrawStringPoint(-4,0),

		FNPrimitiveDrawStringPoint(-4,0),
		FNPrimitiveDrawStringPoint(-2,0),

		FNPrimitiveDrawStringPoint(-2,0),
		FNPrimitiveDrawStringPoint(0,2),
	};

	// K
	Glyphs[75] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0,8),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(-4,4),

		FNPrimitiveDrawStringPoint(-4,4),
		FNPrimitiveDrawStringPoint(-6,6),

		FNPrimitiveDrawStringPoint(-6,6),
		FNPrimitiveDrawStringPoint(-6,8),
		
		FNPrimitiveDrawStringPoint(-4,4),
		FNPrimitiveDrawStringPoint(-6,2),

		FNPrimitiveDrawStringPoint(-6,2),
		FNPrimitiveDrawStringPoint(-6,0)
	};

	// L
	Glyphs[76] = TArray {
		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-6,0)
	};

	// M
	Glyphs[77] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0,8),

		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(-3,4),

		FNPrimitiveDrawStringPoint(-3,4),
		FNPrimitiveDrawStringPoint(-6,8),

		FNPrimitiveDrawStringPoint(-6,8),
		FNPrimitiveDrawStringPoint(-6,0)
	};
	
	// N
	Glyphs[78] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0,8),

		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(-6,0),

		FNPrimitiveDrawStringPoint(-6,0),
		FNPrimitiveDrawStringPoint(-6,8)
	};

	// O
	Glyphs[79] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0, 8),
	
		FNPrimitiveDrawStringPoint(0, 8),
		FNPrimitiveDrawStringPoint(-6, 8),

		FNPrimitiveDrawStringPoint(-6, 8),
		FNPrimitiveDrawStringPoint(-6, 0),

		FNPrimitiveDrawStringPoint(-6, 0),
		FNPrimitiveDrawStringPoint(0, 0)
	};

	// P
	Glyphs[80] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0, 8),

		FNPrimitiveDrawStringPoint(0, 8),
		FNPrimitiveDrawStringPoint(-6, 8),

		FNPrimitiveDrawStringPoint(-6, 8),
		FNPrimitiveDrawStringPoint(-6, 4),

		FNPrimitiveDrawStringPoint(-6, 4),
		FNPrimitiveDrawStringPoint(0, 4)
	};

	// Q
	Glyphs[81] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0, 8),
	
		FNPrimitiveDrawStringPoint(0, 8),
		FNPrimitiveDrawStringPoint(-6, 8),

		FNPrimitiveDrawStringPoint(-6, 8),
		FNPrimitiveDrawStringPoint(-6, 0),

		FNPrimitiveDrawStringPoint(-6, 0),
		FNPrimitiveDrawStringPoint(0, 0),

		FNPrimitiveDrawStringPoint(-6, 0),
		FNPrimitiveDrawStringPoint(-4, 2)
	};

	// R
	Glyphs[82] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0, 8),

		FNPrimitiveDrawStringPoint(0, 8),
		FNPrimitiveDrawStringPoint(-6, 8),

		FNPrimitiveDrawStringPoint(-6, 8),
		FNPrimitiveDrawStringPoint(-6, 4),

		FNPrimitiveDrawStringPoint(-6, 4),
		FNPrimitiveDrawStringPoint(0, 4),

		FNPrimitiveDrawStringPoint(0, 4),
		FNPrimitiveDrawStringPoint(-6, 0)
	};

	// S
	Glyphs[83] = TArray {
		FNPrimitiveDrawStringPoint(-6,8),
		FNPrimitiveDrawStringPoint(0,8),

		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(0,4),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(-6,4),

		FNPrimitiveDrawStringPoint(-6,4),
		FNPrimitiveDrawStringPoint(-6,0),

		FNPrimitiveDrawStringPoint(-6,0),
		FNPrimitiveDrawStringPoint(0,0)
	};

	// T
	Glyphs[84] = TArray {
		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(-6,8),

		FNPrimitiveDrawStringPoint(-3,8),
		FNPrimitiveDrawStringPoint(-3,0)
	};

	// U
	Glyphs[85] = TArray {
		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-6,0),

		FNPrimitiveDrawStringPoint(-6,0),
		FNPrimitiveDrawStringPoint(-6,8)
	};

	// V
	Glyphs[86] = TArray {
		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(-3,0),

		FNPrimitiveDrawStringPoint(-3,0),
		FNPrimitiveDrawStringPoint(-6,8)
	};

	// W
	Glyphs[87] = TArray {
		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-3,3),

		FNPrimitiveDrawStringPoint(-3,3),
		FNPrimitiveDrawStringPoint(-6,0),

		FNPrimitiveDrawStringPoint(-6,0),
		FNPrimitiveDrawStringPoint(-6,8)
	};

	// X
	Glyphs[88] = TArray {
		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(-6,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-6,8)
	};

	// Y
	Glyphs[89] = TArray {
		FNPrimitiveDrawStringPoint(-3,0),
		FNPrimitiveDrawStringPoint(-3,4),

		FNPrimitiveDrawStringPoint(-3,4),
		FNPrimitiveDrawStringPoint(0,8),

		FNPrimitiveDrawStringPoint(-3,4),
		FNPrimitiveDrawStringPoint(-6,8)
	};

	// Z
	Glyphs[90] = TArray {
		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(-6,8),

		FNPrimitiveDrawStringPoint(-6,8),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-6,0)
	};

	// [
	Glyphs[91] = TArray {
		FNPrimitiveDrawStringPoint(-4,8),
		FNPrimitiveDrawStringPoint(0,8),

		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-4,0)
	};

	//  Backslash
	Glyphs[92] = TArray {
		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(-7,0),
	};

	// ]
	Glyphs[93] = TArray {
		FNPrimitiveDrawStringPoint(-2,8),
		FNPrimitiveDrawStringPoint(-6,8),

		FNPrimitiveDrawStringPoint(-6,8),
		FNPrimitiveDrawStringPoint(-6,0),

		FNPrimitiveDrawStringPoint(-6,0),
		FNPrimitiveDrawStringPoint(-2,0)
	};

	// _
	Glyphs[95] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-6,0)
	};

	// a
	Glyphs[97] = TArray {
		FNPrimitiveDrawStringPoint(-6,0),
		FNPrimitiveDrawStringPoint(-6,6),

		FNPrimitiveDrawStringPoint(-6,2),
		FNPrimitiveDrawStringPoint(-4,0),

		FNPrimitiveDrawStringPoint(-4,0),
		FNPrimitiveDrawStringPoint(-2,0),

		FNPrimitiveDrawStringPoint(-2,0),
		FNPrimitiveDrawStringPoint(0,2),

		FNPrimitiveDrawStringPoint(0,2),
		FNPrimitiveDrawStringPoint(0,4),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(-2,6),

		FNPrimitiveDrawStringPoint(-2,6),
		FNPrimitiveDrawStringPoint(-4,6),

		FNPrimitiveDrawStringPoint(-4,6),
		FNPrimitiveDrawStringPoint(-6,4)
	};

	// b
	Glyphs[98] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0,8),

		FNPrimitiveDrawStringPoint(0,2),
		FNPrimitiveDrawStringPoint(-2,0),

		FNPrimitiveDrawStringPoint(-2,0),
		FNPrimitiveDrawStringPoint(-4,0),

		FNPrimitiveDrawStringPoint(-4,0),
		FNPrimitiveDrawStringPoint(-6,2),

		FNPrimitiveDrawStringPoint(-6,2),
		FNPrimitiveDrawStringPoint(-6,4),

		FNPrimitiveDrawStringPoint(-6,4),
		FNPrimitiveDrawStringPoint(-4,6),

		FNPrimitiveDrawStringPoint(-4,6),
		FNPrimitiveDrawStringPoint(-2,6),

		FNPrimitiveDrawStringPoint(-2,6),
		FNPrimitiveDrawStringPoint(0,4)
	};

	// c
	Glyphs[99] = TArray {
		FNPrimitiveDrawStringPoint(-6,0),
		FNPrimitiveDrawStringPoint(-2,0),

		FNPrimitiveDrawStringPoint(-2,0),
		FNPrimitiveDrawStringPoint(0,2),

		FNPrimitiveDrawStringPoint(0,2),
		FNPrimitiveDrawStringPoint(0,4),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(-2,6),

		FNPrimitiveDrawStringPoint(-2,6),
		FNPrimitiveDrawStringPoint(-6,6)
	};

	// d
	Glyphs[100] = TArray {
		FNPrimitiveDrawStringPoint(-6,0),
		FNPrimitiveDrawStringPoint(-6,8),

		FNPrimitiveDrawStringPoint(-6,2),
		FNPrimitiveDrawStringPoint(-4,0),

		FNPrimitiveDrawStringPoint(-4,0),
		FNPrimitiveDrawStringPoint(-2,0),

		FNPrimitiveDrawStringPoint(-2,0),
		FNPrimitiveDrawStringPoint(0,2),

		FNPrimitiveDrawStringPoint(0,2),
		FNPrimitiveDrawStringPoint(0,4),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(-2,6),

		FNPrimitiveDrawStringPoint(-2,6),
		FNPrimitiveDrawStringPoint(-4,6),

		FNPrimitiveDrawStringPoint(-4,6),
		FNPrimitiveDrawStringPoint(-6,4)
	};

	// e
	Glyphs[101] = TArray {
		FNPrimitiveDrawStringPoint(-6,0),
		FNPrimitiveDrawStringPoint(-2,0),

		FNPrimitiveDrawStringPoint(-2,0),
		FNPrimitiveDrawStringPoint(0,2),

		FNPrimitiveDrawStringPoint(0,2),
		FNPrimitiveDrawStringPoint(0,4),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(-2,6),

		FNPrimitiveDrawStringPoint(-2,6),
		FNPrimitiveDrawStringPoint(-4,6),

		FNPrimitiveDrawStringPoint(-4,6),
		FNPrimitiveDrawStringPoint(-6,4),

		FNPrimitiveDrawStringPoint(-6,4),
		FNPrimitiveDrawStringPoint(-6,2),

		FNPrimitiveDrawStringPoint(-6,2),
		FNPrimitiveDrawStringPoint(0,2)
	};

	// f
	Glyphs[102] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0,6),

		FNPrimitiveDrawStringPoint(0,6),
		FNPrimitiveDrawStringPoint(-2,8),

		FNPrimitiveDrawStringPoint(-2,8),
		FNPrimitiveDrawStringPoint(-4,8),

		FNPrimitiveDrawStringPoint(-4,8),
		FNPrimitiveDrawStringPoint(-6,6),

		FNPrimitiveDrawStringPoint(0,3),
		FNPrimitiveDrawStringPoint(-3,3)
	};
	
	// g
	Glyphs[103] = TArray {
		FNPrimitiveDrawStringPoint(0,2),
		FNPrimitiveDrawStringPoint(-2,0),

		FNPrimitiveDrawStringPoint(-2,0),
		FNPrimitiveDrawStringPoint(-4,0),

		FNPrimitiveDrawStringPoint(-4,0),
		FNPrimitiveDrawStringPoint(-6,2),

		FNPrimitiveDrawStringPoint(-6,4),
		FNPrimitiveDrawStringPoint(-4,6),

		FNPrimitiveDrawStringPoint(-4,6),
		FNPrimitiveDrawStringPoint(-2,6),

		FNPrimitiveDrawStringPoint(-2,6),
		FNPrimitiveDrawStringPoint(0,4),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(0,2),

		FNPrimitiveDrawStringPoint(-6,4),
		FNPrimitiveDrawStringPoint(-6,-3),
		
		FNPrimitiveDrawStringPoint(-6,-3),
		FNPrimitiveDrawStringPoint(-2,-3),

		FNPrimitiveDrawStringPoint(-2,-3),
		FNPrimitiveDrawStringPoint(0,-1),
	};
	
	// h
	Glyphs[104] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0,8),
		
		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(-2,6),

		FNPrimitiveDrawStringPoint(-2,6),
		FNPrimitiveDrawStringPoint(-4,6),

		FNPrimitiveDrawStringPoint(-4,6),
		FNPrimitiveDrawStringPoint(-6,4),

		FNPrimitiveDrawStringPoint(-6,4),
		FNPrimitiveDrawStringPoint(-6,0)
	};
	
	// i
	Glyphs[105] = TArray {
		FNPrimitiveDrawStringPoint(-3,7),
		FNPrimitiveDrawStringPoint(-4,6),

		FNPrimitiveDrawStringPoint(-4,6),
		FNPrimitiveDrawStringPoint(-3,5),

		FNPrimitiveDrawStringPoint(-3,5),
		FNPrimitiveDrawStringPoint(-2,6),

		FNPrimitiveDrawStringPoint(-2,6),
		FNPrimitiveDrawStringPoint(-3,7),

		FNPrimitiveDrawStringPoint(-3,4),
		FNPrimitiveDrawStringPoint(-3,0)
	};
	
	// j
	Glyphs[106] = TArray {

		FNPrimitiveDrawStringPoint(-3,7),
		FNPrimitiveDrawStringPoint(-4,6),

		FNPrimitiveDrawStringPoint(-4,6),
		FNPrimitiveDrawStringPoint(-3,5),

		FNPrimitiveDrawStringPoint(-3,5),
		FNPrimitiveDrawStringPoint(-2,6),

		FNPrimitiveDrawStringPoint(-2,6),
		FNPrimitiveDrawStringPoint(-3,7),
		
		FNPrimitiveDrawStringPoint(-3,4),
		FNPrimitiveDrawStringPoint(-3,-1),

		FNPrimitiveDrawStringPoint(-3,-1),
		FNPrimitiveDrawStringPoint(-2,-2),

		FNPrimitiveDrawStringPoint(-2,-2),
		FNPrimitiveDrawStringPoint(-1,-2),

		
	};
	
	// k
	Glyphs[107] = TArray {
		FNPrimitiveDrawStringPoint(0,8),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,2),
		FNPrimitiveDrawStringPoint(-4,2),

		FNPrimitiveDrawStringPoint(-4,2),
		FNPrimitiveDrawStringPoint(-6,0),

		FNPrimitiveDrawStringPoint(-2,2),
		FNPrimitiveDrawStringPoint(-4,4),

		FNPrimitiveDrawStringPoint(-4,4),
		FNPrimitiveDrawStringPoint(-4,6)
	};

	// l
	Glyphs[108] = TArray {
		FNPrimitiveDrawStringPoint(-3,8),
		FNPrimitiveDrawStringPoint(-3,1),
				
		FNPrimitiveDrawStringPoint(-3,1),
		FNPrimitiveDrawStringPoint(-4,0)
	};
	
	// m
	Glyphs[109] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0,4),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(-2,6),

		FNPrimitiveDrawStringPoint(-2,6),
		FNPrimitiveDrawStringPoint(-3,4),

		FNPrimitiveDrawStringPoint(-3,4),
		FNPrimitiveDrawStringPoint(-4,6),

		FNPrimitiveDrawStringPoint(-4,6),
		FNPrimitiveDrawStringPoint(-6,4),

		FNPrimitiveDrawStringPoint(-6,4),
		FNPrimitiveDrawStringPoint(-6,0)
	};
	
	// n
	Glyphs[110] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0,6),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(-2,6),

		FNPrimitiveDrawStringPoint(-2,6),
		FNPrimitiveDrawStringPoint(-4,6),

		FNPrimitiveDrawStringPoint(-4,6),
		FNPrimitiveDrawStringPoint(-6,4),

		FNPrimitiveDrawStringPoint(-6,4),
		FNPrimitiveDrawStringPoint(-6,0)
	};
	
	// o
	Glyphs[111] = TArray {
		FNPrimitiveDrawStringPoint(0,2),
		FNPrimitiveDrawStringPoint(-2,0),

		FNPrimitiveDrawStringPoint(-2,0),
		FNPrimitiveDrawStringPoint(-4,0),

		FNPrimitiveDrawStringPoint(-4,0),
		FNPrimitiveDrawStringPoint(-6,2),

		FNPrimitiveDrawStringPoint(-6,2),
		FNPrimitiveDrawStringPoint(-6,4),

		FNPrimitiveDrawStringPoint(-6,4),
		FNPrimitiveDrawStringPoint(-4,6),

		FNPrimitiveDrawStringPoint(-4,6),
		FNPrimitiveDrawStringPoint(-2,6),

		FNPrimitiveDrawStringPoint(-2,6),
		FNPrimitiveDrawStringPoint(0,4),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(0,2)
	};
	
	// p
	Glyphs[112] = TArray {
		FNPrimitiveDrawStringPoint(0,2),
		FNPrimitiveDrawStringPoint(-2,0),

		FNPrimitiveDrawStringPoint(-2,0),
		FNPrimitiveDrawStringPoint(-4,0),

		FNPrimitiveDrawStringPoint(-4,0),
		FNPrimitiveDrawStringPoint(-6,2),

		FNPrimitiveDrawStringPoint(-6,2),
		FNPrimitiveDrawStringPoint(-6,4),

		FNPrimitiveDrawStringPoint(-6,4),
		FNPrimitiveDrawStringPoint(-4,6),

		FNPrimitiveDrawStringPoint(-4,6),
		FNPrimitiveDrawStringPoint(-2,6),

		FNPrimitiveDrawStringPoint(-2,6),
		FNPrimitiveDrawStringPoint(0,4),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(0,-2)
	};
	
	// q
	Glyphs[113] = TArray {
		FNPrimitiveDrawStringPoint(0,2),
		FNPrimitiveDrawStringPoint(-2,0),

		FNPrimitiveDrawStringPoint(-2,0),
		FNPrimitiveDrawStringPoint(-4,0),

		FNPrimitiveDrawStringPoint(-4,0),
		FNPrimitiveDrawStringPoint(-6,2),

		FNPrimitiveDrawStringPoint(-6,4),
		FNPrimitiveDrawStringPoint(-4,6),

		FNPrimitiveDrawStringPoint(-4,6),
		FNPrimitiveDrawStringPoint(-2,6),

		FNPrimitiveDrawStringPoint(-2,6),
		FNPrimitiveDrawStringPoint(0,4),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(0,2),

		FNPrimitiveDrawStringPoint(-6,-2),
		FNPrimitiveDrawStringPoint(-6,4)
	};
	
	// r
	Glyphs[114] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(0,4),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(-2,6),

		FNPrimitiveDrawStringPoint(-2,6),
		FNPrimitiveDrawStringPoint(-4,6),

		FNPrimitiveDrawStringPoint(-4,6),
		FNPrimitiveDrawStringPoint(-6,4)
	};
	
	// s
	Glyphs[115] = TArray {
		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-3,0),

		FNPrimitiveDrawStringPoint(-3,0),
		FNPrimitiveDrawStringPoint(-5,1),

		FNPrimitiveDrawStringPoint(-5,1),
		FNPrimitiveDrawStringPoint(-6,2),

		FNPrimitiveDrawStringPoint(-6,2),
		FNPrimitiveDrawStringPoint(-6,3),
		
		FNPrimitiveDrawStringPoint(-6,3),
		FNPrimitiveDrawStringPoint(-5,4),

		FNPrimitiveDrawStringPoint(-5,4),
		FNPrimitiveDrawStringPoint(0,4),

		FNPrimitiveDrawStringPoint(0,4),
		FNPrimitiveDrawStringPoint(0,5),

		FNPrimitiveDrawStringPoint(0,5),
		FNPrimitiveDrawStringPoint(-2,6),

		FNPrimitiveDrawStringPoint(-2,6),
		FNPrimitiveDrawStringPoint(-5,6),

		FNPrimitiveDrawStringPoint(-5,6),
		FNPrimitiveDrawStringPoint(-6,5)
	};
	
	// t
	Glyphs[116] = TArray {
		FNPrimitiveDrawStringPoint(-3,0),
		FNPrimitiveDrawStringPoint(-3,8),

		FNPrimitiveDrawStringPoint(-1,4),
		FNPrimitiveDrawStringPoint(-5,4)
	};
	
	// u
	Glyphs[117] = TArray {
		FNPrimitiveDrawStringPoint(0,6),
		FNPrimitiveDrawStringPoint(0,2),

		FNPrimitiveDrawStringPoint(0,2),
		FNPrimitiveDrawStringPoint(-2,0),

		FNPrimitiveDrawStringPoint(-2,0),
		FNPrimitiveDrawStringPoint(-4,0),

		FNPrimitiveDrawStringPoint(-4,0),
		FNPrimitiveDrawStringPoint(-6,2),

		FNPrimitiveDrawStringPoint(-6,2),
		FNPrimitiveDrawStringPoint(-6,6)
	};
	
	// v
	Glyphs[118] = TArray {
		FNPrimitiveDrawStringPoint(0,6),
		FNPrimitiveDrawStringPoint(-3,0),

		FNPrimitiveDrawStringPoint(-3,0),
		FNPrimitiveDrawStringPoint(-6,6)
	};
	
	// w
	Glyphs[119] = TArray {
		FNPrimitiveDrawStringPoint(0,6),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-2,0),

		FNPrimitiveDrawStringPoint(-2,0),
		FNPrimitiveDrawStringPoint(-3,2),
		
		FNPrimitiveDrawStringPoint(-3,2),
		FNPrimitiveDrawStringPoint(-4,0),

		FNPrimitiveDrawStringPoint(-4,0),
		FNPrimitiveDrawStringPoint(-6,0),

		FNPrimitiveDrawStringPoint(-6,0),
		FNPrimitiveDrawStringPoint(-6,6),
	};
	
	// x
	Glyphs[120] = TArray {
		FNPrimitiveDrawStringPoint(0,6),
		FNPrimitiveDrawStringPoint(-6,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-6,6)
	};
	
	// y
	Glyphs[121] = TArray {
		FNPrimitiveDrawStringPoint(0,6),
		FNPrimitiveDrawStringPoint(0,2),

		FNPrimitiveDrawStringPoint(0,2),
		FNPrimitiveDrawStringPoint(-2, 0),

		FNPrimitiveDrawStringPoint(-2, 0),
		FNPrimitiveDrawStringPoint(-4, 0),

		FNPrimitiveDrawStringPoint(-4, 0),
		FNPrimitiveDrawStringPoint(-6, 2),

		FNPrimitiveDrawStringPoint(-6, 6),
		FNPrimitiveDrawStringPoint(-6, -2),

		FNPrimitiveDrawStringPoint(-6, -2),
		FNPrimitiveDrawStringPoint(-3, -3),

		FNPrimitiveDrawStringPoint(-3, -3),
		FNPrimitiveDrawStringPoint(0, -1)
	};
	
	// z
	Glyphs[122] = TArray {
		FNPrimitiveDrawStringPoint(0,6),
		FNPrimitiveDrawStringPoint(-6,6),

		FNPrimitiveDrawStringPoint(-6,6),
		FNPrimitiveDrawStringPoint(0,0),

		FNPrimitiveDrawStringPoint(0,0),
		FNPrimitiveDrawStringPoint(-6,0)
	};
	
	// Flag that we have generated at this point.
	bHasGeneratedGlyphs = true;
}