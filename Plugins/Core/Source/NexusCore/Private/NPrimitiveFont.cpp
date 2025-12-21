// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NPrimitiveFont.h"

bool FNPrimitiveFont::bIsInitialized = false;
TArray<TArray<FNPrimitiveFontPoint>> FNPrimitiveFont::Glyphs = TArray<TArray<FNPrimitiveFontPoint>>();

TArray<FNPrimitiveFontPoint>& FNPrimitiveFont::GetGlyph(const char InChar)
{
	if (InChar < 32 || InChar > 126) return Glyphs[0];
	return Glyphs[InChar];
}

void FNPrimitiveFont::Initialize()
{
	if (bIsInitialized) return;
	
	// A glyph is based off a 6(w)x8(h) bottom-left oriented grid.
	// It should consist of paired points which will be used to draw the corresponding lines.
	Glyphs.AddZeroed(127);
	
	// Replace null with unknown-character as we'll use it.
	Glyphs[0] = TArray {
		FNPrimitiveFontPoint(0, 0),
		FNPrimitiveFontPoint(0, 8),
		
		FNPrimitiveFontPoint(0, 8),
		FNPrimitiveFontPoint(6, 8),

		FNPrimitiveFontPoint(6, 8),
		FNPrimitiveFontPoint(6, 0),

		FNPrimitiveFontPoint(6, 0),
		FNPrimitiveFontPoint(0, 0),

		FNPrimitiveFontPoint(0, 8),
		FNPrimitiveFontPoint(6, 0),

		FNPrimitiveFontPoint(6, 8),
		FNPrimitiveFontPoint(0, 0),
	};

	// !
	Glyphs[33] = TArray {
		FNPrimitiveFontPoint(3, 0),
		FNPrimitiveFontPoint(4, 1),

		FNPrimitiveFontPoint(4, 1),
		FNPrimitiveFontPoint(3, 2),

		FNPrimitiveFontPoint(3, 2),
		FNPrimitiveFontPoint(2, 1),

		FNPrimitiveFontPoint(2, 1),
		FNPrimitiveFontPoint(3, 0),

		FNPrimitiveFontPoint(3, 3),
		FNPrimitiveFontPoint(4, 8),

		FNPrimitiveFontPoint(4, 8),
		FNPrimitiveFontPoint(2, 8),

		FNPrimitiveFontPoint(2, 8),
		FNPrimitiveFontPoint(3, 3)
	};

	// "
	Glyphs[34] = TArray {
		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(2,8),

		FNPrimitiveFontPoint(4,6),
		FNPrimitiveFontPoint(4,8),
	};

	// #
	Glyphs[35] = TArray {
		FNPrimitiveFontPoint(2,7),
		FNPrimitiveFontPoint(2,1),

		FNPrimitiveFontPoint(4,7),
		FNPrimitiveFontPoint(4,1),
		
		FNPrimitiveFontPoint(1,3),
		FNPrimitiveFontPoint(5,3),

		FNPrimitiveFontPoint(1,5),
		FNPrimitiveFontPoint(5,5)
	};

	// $ 
	Glyphs[36] = TArray {
		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(0,4),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(6,4),

		FNPrimitiveFontPoint(6,4),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(3,0),
		FNPrimitiveFontPoint(3,8)
	};
	
	// %
	Glyphs[37] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,8),

		
		FNPrimitiveFontPoint(0,7),
		FNPrimitiveFontPoint(1,8),

		FNPrimitiveFontPoint(1,8),
		FNPrimitiveFontPoint(2,7),

		FNPrimitiveFontPoint(2,7),
		FNPrimitiveFontPoint(1,6),

		FNPrimitiveFontPoint(1,6),
		FNPrimitiveFontPoint(0,7),
		
		FNPrimitiveFontPoint(4,1),
		FNPrimitiveFontPoint(5,2),

		FNPrimitiveFontPoint(5,2),
		FNPrimitiveFontPoint(6,1),

		FNPrimitiveFontPoint(6,1),
		FNPrimitiveFontPoint(5,0),

		FNPrimitiveFontPoint(5,0),
		FNPrimitiveFontPoint(4,1)
	};

	// &
	Glyphs[38] = TArray {
		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(5,0),

		FNPrimitiveFontPoint(5,0),
		FNPrimitiveFontPoint(1,5),

		FNPrimitiveFontPoint(1,5),
		FNPrimitiveFontPoint(1,6),

		FNPrimitiveFontPoint(1,6),
		FNPrimitiveFontPoint(3,8),

		FNPrimitiveFontPoint(3,8),
		FNPrimitiveFontPoint(5,6),

		FNPrimitiveFontPoint(5,6),
		FNPrimitiveFontPoint(5,5),

		FNPrimitiveFontPoint(5,5),
		FNPrimitiveFontPoint(1,3),

		FNPrimitiveFontPoint(1,3),
		FNPrimitiveFontPoint(1,1),

		FNPrimitiveFontPoint(1,1),
		FNPrimitiveFontPoint(2,0),

		FNPrimitiveFontPoint(2,0),
		FNPrimitiveFontPoint(4,0),

		FNPrimitiveFontPoint(4,0),
		FNPrimitiveFontPoint(5,2),

		FNPrimitiveFontPoint(5,2),
		FNPrimitiveFontPoint(5,3)
	};
	
	// '
	Glyphs[39] = TArray {
		FNPrimitiveFontPoint(2,8),
		FNPrimitiveFontPoint(2,6)
	};
	
	// (
	Glyphs[40] = TArray {
		FNPrimitiveFontPoint(4,8),
		FNPrimitiveFontPoint(2,8),

		FNPrimitiveFontPoint(2,8),
		FNPrimitiveFontPoint(0,6),

		FNPrimitiveFontPoint(0,6),
		FNPrimitiveFontPoint(0,2),

		FNPrimitiveFontPoint(0,2),
		FNPrimitiveFontPoint(2,0),

		FNPrimitiveFontPoint(2,0),
		FNPrimitiveFontPoint(4,0)
	};

	// )
	Glyphs[41] = TArray {
		FNPrimitiveFontPoint(2,8),
		FNPrimitiveFontPoint(4,8),

		FNPrimitiveFontPoint(4,8),
		FNPrimitiveFontPoint(6,6),

		FNPrimitiveFontPoint(6,6),
		FNPrimitiveFontPoint(6,2),

		FNPrimitiveFontPoint(6,2),
		FNPrimitiveFontPoint(4,0),

		FNPrimitiveFontPoint(4, 0),
		FNPrimitiveFontPoint(2,0)
	};

	// *
	Glyphs[42] = TArray { 
		FNPrimitiveFontPoint(1,2),
		FNPrimitiveFontPoint(5,6),

		FNPrimitiveFontPoint(1,6),
		FNPrimitiveFontPoint(5,2),

		FNPrimitiveFontPoint(3,2),
		FNPrimitiveFontPoint(3,6)
	};
	
	// +
	Glyphs[43] = TArray { 
		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(6,4),
		
		FNPrimitiveFontPoint(3,7),
		FNPrimitiveFontPoint(3,1)
	};

	// ,
	Glyphs[44] = TArray {
		FNPrimitiveFontPoint(1,1),
		FNPrimitiveFontPoint(1,0),

		FNPrimitiveFontPoint(1,0),
		FNPrimitiveFontPoint(0,-1)
	};
		
	// -
	Glyphs[45] = TArray {
		FNPrimitiveFontPoint(2,4),
		FNPrimitiveFontPoint(4,4)
	};

	// .
	Glyphs[46] = TArray {
		FNPrimitiveFontPoint(0,1),
		FNPrimitiveFontPoint(1,1),

		FNPrimitiveFontPoint(1,1),
		FNPrimitiveFontPoint(1,0),

		FNPrimitiveFontPoint(1,0),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0,1)
	};

	// Forward slash
	Glyphs[47] = TArray {
		FNPrimitiveFontPoint(1,0),
		FNPrimitiveFontPoint(7,8),
	};
	
	// 0
	Glyphs[48] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0, 8),
		
		FNPrimitiveFontPoint(0, 8),
		FNPrimitiveFontPoint(6, 8),

		FNPrimitiveFontPoint(6, 8),
		FNPrimitiveFontPoint(6, 0),

		FNPrimitiveFontPoint(6, 0),
		FNPrimitiveFontPoint(0, 0),

		FNPrimitiveFontPoint(6, 8),
		FNPrimitiveFontPoint(0, 0)
	};

	// 1
	Glyphs[49] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,0),
		
		FNPrimitiveFontPoint(3,0),
		FNPrimitiveFontPoint(3,8),

		FNPrimitiveFontPoint(3,8),
		FNPrimitiveFontPoint(0,6)
	};

	// 2
	Glyphs[50] = TArray {
		FNPrimitiveFontPoint(0,6),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(6,8),

		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(6,6),

		FNPrimitiveFontPoint(6,6),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,0),
	};

	// 3
	Glyphs[51] = TArray {
		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(6,8),

		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(2,4),
		FNPrimitiveFontPoint(6,4),
	};

	// 4
	Glyphs[52] = TArray{
		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(6,8),

		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(0,4),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(6,4),
	};

	// 5
	Glyphs[53] = TArray {
		FNPrimitiveFontPoint(0,2),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(6,4),

		FNPrimitiveFontPoint(6,4),
		FNPrimitiveFontPoint(0,4),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(0,8),
		
		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(6,8)
	};

	// 6
	Glyphs[54] = TArray {
		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(6,4),

		FNPrimitiveFontPoint(6,4),
		FNPrimitiveFontPoint(0,4)
	};

	// 7
	Glyphs[55] = TArray {
		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(6,8),

		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(0,0)
	};

	// 8
	Glyphs[56] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0, 8),

		FNPrimitiveFontPoint(0, 8),
		FNPrimitiveFontPoint(6, 8),

		FNPrimitiveFontPoint(6, 8),
		FNPrimitiveFontPoint(6, 0),

		FNPrimitiveFontPoint(6, 0),
		FNPrimitiveFontPoint(0, 0),

		FNPrimitiveFontPoint(0, 4),
		FNPrimitiveFontPoint(6, 4)
	};

	// 9
	Glyphs[57] = TArray {
		FNPrimitiveFontPoint(0,2),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(6,8),

		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(0,4),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(6,4)
	};

	// :
	Glyphs[58] = TArray {
		FNPrimitiveFontPoint(2, 1),
		FNPrimitiveFontPoint(4, 1),

		FNPrimitiveFontPoint(4, 1),
		FNPrimitiveFontPoint(4, 3),

		FNPrimitiveFontPoint(4, 3),
		FNPrimitiveFontPoint(2, 3),

		FNPrimitiveFontPoint(2, 3),
		FNPrimitiveFontPoint(2, 1),

		FNPrimitiveFontPoint(2, 5),
		FNPrimitiveFontPoint(4, 5),

		FNPrimitiveFontPoint(4, 5),
		FNPrimitiveFontPoint(4, 7),

		FNPrimitiveFontPoint(4, 7),
		FNPrimitiveFontPoint(2, 7),

		FNPrimitiveFontPoint(2, 7),
		FNPrimitiveFontPoint(2, 5)
	};

	// ;
	Glyphs[59] = TArray { 
		FNPrimitiveFontPoint(1,1),
		FNPrimitiveFontPoint(1,0),

		FNPrimitiveFontPoint(1,0),
		FNPrimitiveFontPoint(0,-1),
	
		FNPrimitiveFontPoint(1,2),
		FNPrimitiveFontPoint(2,3),

		FNPrimitiveFontPoint(2,3),
		FNPrimitiveFontPoint(1,4),

		FNPrimitiveFontPoint(1,4),
		FNPrimitiveFontPoint(0,3),

		FNPrimitiveFontPoint(0,3),
		FNPrimitiveFontPoint(1,2)
	};

	// <
	Glyphs[60] = TArray {
		FNPrimitiveFontPoint(5,7),
		FNPrimitiveFontPoint(1,4),

		FNPrimitiveFontPoint(1,4),
		FNPrimitiveFontPoint(5,1)
	};

	// =
	Glyphs[61] = TArray { 
		FNPrimitiveFontPoint(0,5),
		FNPrimitiveFontPoint(6,5),

		FNPrimitiveFontPoint(0,3),
		FNPrimitiveFontPoint(6,3)
	};
	
	// >
	Glyphs[62] = TArray { 
		FNPrimitiveFontPoint(1,7),
		FNPrimitiveFontPoint(5,4),
		
		FNPrimitiveFontPoint(5,4),
		FNPrimitiveFontPoint(1,1)
	};	

	// ?
	Glyphs[63] = TArray {
		FNPrimitiveFontPoint(3,0),
		FNPrimitiveFontPoint(4,1),

		FNPrimitiveFontPoint(4,1),
		FNPrimitiveFontPoint(3,2),

		FNPrimitiveFontPoint(3,2),
		FNPrimitiveFontPoint(2,1),

		FNPrimitiveFontPoint(2,1),
		FNPrimitiveFontPoint(3,0),
		
		FNPrimitiveFontPoint(3,3),
		FNPrimitiveFontPoint(3,4),
		
		FNPrimitiveFontPoint(3,4),
		FNPrimitiveFontPoint(5,5),
		
		FNPrimitiveFontPoint(5,5),
		FNPrimitiveFontPoint(5,6),
		
		FNPrimitiveFontPoint(5,6),
		FNPrimitiveFontPoint(4,8),
		
		FNPrimitiveFontPoint(4,8),
		FNPrimitiveFontPoint(2,8),
		
		FNPrimitiveFontPoint(2,8),
		FNPrimitiveFontPoint(1,6)
	};
	
	// @
	Glyphs[64] = TArray { 
		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(6,8),

		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(6,2),

		FNPrimitiveFontPoint(6,2),
		FNPrimitiveFontPoint(2,2),

		FNPrimitiveFontPoint(2,2),
		FNPrimitiveFontPoint(2,6),

		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(4,6)
	};
	
	// A
	Glyphs[65] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0, 8),
		
		FNPrimitiveFontPoint(0, 8),
		FNPrimitiveFontPoint(6, 8),

		FNPrimitiveFontPoint(6, 8),
		FNPrimitiveFontPoint(6, 0),

		FNPrimitiveFontPoint(0, 4),
		FNPrimitiveFontPoint(6, 4)
	};
	
	// B
	Glyphs[66] = TArray {
		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(4,4),

		FNPrimitiveFontPoint(4,4),
		FNPrimitiveFontPoint(6,6),
		
		FNPrimitiveFontPoint(6,6),
		FNPrimitiveFontPoint(6,8),

		FNPrimitiveFontPoint(4,4),
		FNPrimitiveFontPoint(6,2),
		
		FNPrimitiveFontPoint(6,2),
		FNPrimitiveFontPoint(6,0)
	};

	// C
	Glyphs[67] = TArray {
		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,0)
	};

	// D
	Glyphs[68] = TArray {
		FNPrimitiveFontPoint(4,8),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(4,0),

		FNPrimitiveFontPoint(4,8),
		FNPrimitiveFontPoint(6,6),

		FNPrimitiveFontPoint(6,6),
		FNPrimitiveFontPoint(6,2),

		FNPrimitiveFontPoint(6,2),
		FNPrimitiveFontPoint(4,0)
	};

	// E
	Glyphs[69] = TArray {
		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(4,4)
	};

	// F
	Glyphs[70] = TArray {
		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(4,4)
	};

	// G
	Glyphs[71] = TArray {
		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(6,4),

		FNPrimitiveFontPoint(6,4),
		FNPrimitiveFontPoint(4,4)
	};

	// H
	Glyphs[72] = TArray {
		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(0,0),
		
		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(6,4)
	};

	// I
	Glyphs[73] = TArray {
		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(6,8),

		FNPrimitiveFontPoint(3, 8),
		FNPrimitiveFontPoint(3, 0),
		
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,0),
	};

	// J
	Glyphs[74] = TArray {
		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(6,8),

		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(6,2),

		FNPrimitiveFontPoint(6,2),
		FNPrimitiveFontPoint(4,0),

		FNPrimitiveFontPoint(4,0),
		FNPrimitiveFontPoint(2,0),

		FNPrimitiveFontPoint(2,0),
		FNPrimitiveFontPoint(0,2),
	};

	// K
	Glyphs[75] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(4,4),

		FNPrimitiveFontPoint(4,4),
		FNPrimitiveFontPoint(6,6),

		FNPrimitiveFontPoint(6,6),
		FNPrimitiveFontPoint(6,8),
		
		FNPrimitiveFontPoint(4,4),
		FNPrimitiveFontPoint(6,2),

		FNPrimitiveFontPoint(6,2),
		FNPrimitiveFontPoint(6,0)
	};

	// L
	Glyphs[76] = TArray {
		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,0)
	};

	// M
	Glyphs[77] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(3,4),

		FNPrimitiveFontPoint(3,4),
		FNPrimitiveFontPoint(6,8),

		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(6,0)
	};
	
	// N
	Glyphs[78] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(6,8)
	};

	// O
	Glyphs[79] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0, 8),
	
		FNPrimitiveFontPoint(0, 8),
		FNPrimitiveFontPoint(6, 8),

		FNPrimitiveFontPoint(6, 8),
		FNPrimitiveFontPoint(6, 0),

		FNPrimitiveFontPoint(6, 0),
		FNPrimitiveFontPoint(0, 0)
	};

	// P
	Glyphs[80] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0, 8),

		FNPrimitiveFontPoint(0, 8),
		FNPrimitiveFontPoint(6, 8),

		FNPrimitiveFontPoint(6, 8),
		FNPrimitiveFontPoint(6, 4),

		FNPrimitiveFontPoint(6, 4),
		FNPrimitiveFontPoint(0, 4)
	};

	// Q
	Glyphs[81] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0, 8),
	
		FNPrimitiveFontPoint(0, 8),
		FNPrimitiveFontPoint(6, 8),

		FNPrimitiveFontPoint(6, 8),
		FNPrimitiveFontPoint(6, 0),

		FNPrimitiveFontPoint(6, 0),
		FNPrimitiveFontPoint(0, 0),

		FNPrimitiveFontPoint(6, 0),
		FNPrimitiveFontPoint(4, 2)
	};

	// R
	Glyphs[82] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0, 8),

		FNPrimitiveFontPoint(0, 8),
		FNPrimitiveFontPoint(6, 8),

		FNPrimitiveFontPoint(6, 8),
		FNPrimitiveFontPoint(6, 4),

		FNPrimitiveFontPoint(6, 4),
		FNPrimitiveFontPoint(0, 4),

		FNPrimitiveFontPoint(0, 4),
		FNPrimitiveFontPoint(6, 0)
	};

	// S
	Glyphs[83] = TArray {
		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(0,4),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(6,4),

		FNPrimitiveFontPoint(6,4),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(0,0)
	};

	// T
	Glyphs[84] = TArray {
		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(6,8),

		FNPrimitiveFontPoint(3,8),
		FNPrimitiveFontPoint(3,0)
	};

	// U
	Glyphs[85] = TArray {
		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(6,8)
	};

	// V
	Glyphs[86] = TArray {
		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(3,0),

		FNPrimitiveFontPoint(3,0),
		FNPrimitiveFontPoint(6,8)
	};

	// W
	Glyphs[87] = TArray {
		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(3,3),

		FNPrimitiveFontPoint(3,3),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(6,8)
	};

	// X
	Glyphs[88] = TArray {
		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,8)
	};

	// Y
	Glyphs[89] = TArray {
		FNPrimitiveFontPoint(3,0),
		FNPrimitiveFontPoint(3,4),

		FNPrimitiveFontPoint(3,4),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(3,4),
		FNPrimitiveFontPoint(6,8)
	};

	// Z
	Glyphs[90] = TArray {
		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(6,8),

		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,0)
	};

	// [
	Glyphs[91] = TArray {
		FNPrimitiveFontPoint(4,8),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(4,0)
	};

	//  Backslash
	Glyphs[92] = TArray {
		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(7,0),
	};

	// ]
	Glyphs[93] = TArray {
		FNPrimitiveFontPoint(2,8),
		FNPrimitiveFontPoint(6,8),

		FNPrimitiveFontPoint(6,8),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(2,0)
	};
	
	// ^
	Glyphs[94] = TArray {
		FNPrimitiveFontPoint(1,5),
		FNPrimitiveFontPoint(3,8),
		
		FNPrimitiveFontPoint(3,8),
		FNPrimitiveFontPoint(5,5),
	};

	// _
	Glyphs[95] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,0)
	};

	// `
	Glyphs[96] = TArray {
		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(1,6)
	};
	
	// a
	Glyphs[97] = TArray {
		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(6,6),

		FNPrimitiveFontPoint(6,2),
		FNPrimitiveFontPoint(4,0),

		FNPrimitiveFontPoint(4,0),
		FNPrimitiveFontPoint(2,0),

		FNPrimitiveFontPoint(2,0),
		FNPrimitiveFontPoint(0,2),

		FNPrimitiveFontPoint(0,2),
		FNPrimitiveFontPoint(0,4),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(2,6),

		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(4,6),

		FNPrimitiveFontPoint(4,6),
		FNPrimitiveFontPoint(6,4)
	};

	// b
	Glyphs[98] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0,8),

		FNPrimitiveFontPoint(0,2),
		FNPrimitiveFontPoint(2,0),

		FNPrimitiveFontPoint(2,0),
		FNPrimitiveFontPoint(4,0),

		FNPrimitiveFontPoint(4,0),
		FNPrimitiveFontPoint(6,2),

		FNPrimitiveFontPoint(6,2),
		FNPrimitiveFontPoint(6,4),

		FNPrimitiveFontPoint(6,4),
		FNPrimitiveFontPoint(4,6),

		FNPrimitiveFontPoint(4,6),
		FNPrimitiveFontPoint(2,6),

		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(0,4)
	};

	// c
	Glyphs[99] = TArray {
		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(2,0),

		FNPrimitiveFontPoint(2,0),
		FNPrimitiveFontPoint(0,2),

		FNPrimitiveFontPoint(0,2),
		FNPrimitiveFontPoint(0,4),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(2,6),

		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(6,6)
	};

	// d
	Glyphs[100] = TArray {
		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(6,8),

		FNPrimitiveFontPoint(6,2),
		FNPrimitiveFontPoint(4,0),

		FNPrimitiveFontPoint(4,0),
		FNPrimitiveFontPoint(2,0),

		FNPrimitiveFontPoint(2,0),
		FNPrimitiveFontPoint(0,2),

		FNPrimitiveFontPoint(0,2),
		FNPrimitiveFontPoint(0,4),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(2,6),

		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(4,6),

		FNPrimitiveFontPoint(4,6),
		FNPrimitiveFontPoint(6,4)
	};

	// e
	Glyphs[101] = TArray {
		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(2,0),

		FNPrimitiveFontPoint(2,0),
		FNPrimitiveFontPoint(0,2),

		FNPrimitiveFontPoint(0,2),
		FNPrimitiveFontPoint(0,4),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(2,6),

		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(4,6),

		FNPrimitiveFontPoint(4,6),
		FNPrimitiveFontPoint(6,4),

		FNPrimitiveFontPoint(6,4),
		FNPrimitiveFontPoint(6,2),

		FNPrimitiveFontPoint(6,2),
		FNPrimitiveFontPoint(0,2)
	};

	// f
	Glyphs[102] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0,6),

		FNPrimitiveFontPoint(0,6),
		FNPrimitiveFontPoint(2,8),

		FNPrimitiveFontPoint(2,8),
		FNPrimitiveFontPoint(4,8),

		FNPrimitiveFontPoint(4,8),
		FNPrimitiveFontPoint(6,6),

		FNPrimitiveFontPoint(0,3),
		FNPrimitiveFontPoint(3,3)
	};
	
	// g
	Glyphs[103] = TArray {
		FNPrimitiveFontPoint(0,2),
		FNPrimitiveFontPoint(2,0),

		FNPrimitiveFontPoint(2,0),
		FNPrimitiveFontPoint(4,0),

		FNPrimitiveFontPoint(4,0),
		FNPrimitiveFontPoint(6,2),

		FNPrimitiveFontPoint(6,4),
		FNPrimitiveFontPoint(4,6),

		FNPrimitiveFontPoint(4,6),
		FNPrimitiveFontPoint(2,6),

		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(0,4),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(0,2),

		FNPrimitiveFontPoint(6,4),
		FNPrimitiveFontPoint(6,-3),
		
		FNPrimitiveFontPoint(6,-3),
		FNPrimitiveFontPoint(2,-3),

		FNPrimitiveFontPoint(2,-3),
		FNPrimitiveFontPoint(0,-1),
	};
	
	// h
	Glyphs[104] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0,8),
		
		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(2,6),

		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(4,6),

		FNPrimitiveFontPoint(4,6),
		FNPrimitiveFontPoint(6,4),

		FNPrimitiveFontPoint(6,4),
		FNPrimitiveFontPoint(6,0)
	};
	
	// i
	Glyphs[105] = TArray {
		FNPrimitiveFontPoint(3,7),
		FNPrimitiveFontPoint(4,6),

		FNPrimitiveFontPoint(4,6),
		FNPrimitiveFontPoint(3,5),

		FNPrimitiveFontPoint(3,5),
		FNPrimitiveFontPoint(2,6),

		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(3,7),

		FNPrimitiveFontPoint(3,4),
		FNPrimitiveFontPoint(3,0)
	};
	
	// j
	Glyphs[106] = TArray {

		FNPrimitiveFontPoint(3,7),
		FNPrimitiveFontPoint(4,6),

		FNPrimitiveFontPoint(4,6),
		FNPrimitiveFontPoint(3,5),

		FNPrimitiveFontPoint(3,5),
		FNPrimitiveFontPoint(2,6),

		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(3,7),
		
		FNPrimitiveFontPoint(3,4),
		FNPrimitiveFontPoint(3,-1),

		FNPrimitiveFontPoint(3,-1),
		FNPrimitiveFontPoint(2,-2),

		FNPrimitiveFontPoint(2,-2),
		FNPrimitiveFontPoint(1,-2),

		
	};
	
	// k
	Glyphs[107] = TArray {
		FNPrimitiveFontPoint(0,8),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,2),
		FNPrimitiveFontPoint(4,2),

		FNPrimitiveFontPoint(4,2),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(2,2),
		FNPrimitiveFontPoint(4,4),

		FNPrimitiveFontPoint(4,4),
		FNPrimitiveFontPoint(4,6)
	};

	// l
	Glyphs[108] = TArray {
		FNPrimitiveFontPoint(3,8),
		FNPrimitiveFontPoint(3,1),
				
		FNPrimitiveFontPoint(3,1),
		FNPrimitiveFontPoint(4,0)
	};
	
	// m
	Glyphs[109] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0,4),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(2,6),

		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(3,4),

		FNPrimitiveFontPoint(3,4),
		FNPrimitiveFontPoint(4,6),

		FNPrimitiveFontPoint(4,6),
		FNPrimitiveFontPoint(6,4),

		FNPrimitiveFontPoint(6,4),
		FNPrimitiveFontPoint(6,0)
	};
	
	// n
	Glyphs[110] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0,6),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(2,6),

		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(4,6),

		FNPrimitiveFontPoint(4,6),
		FNPrimitiveFontPoint(6,4),

		FNPrimitiveFontPoint(6,4),
		FNPrimitiveFontPoint(6,0)
	};
	
	// o
	Glyphs[111] = TArray {
		FNPrimitiveFontPoint(0,2),
		FNPrimitiveFontPoint(2,0),

		FNPrimitiveFontPoint(2,0),
		FNPrimitiveFontPoint(4,0),

		FNPrimitiveFontPoint(4,0),
		FNPrimitiveFontPoint(6,2),

		FNPrimitiveFontPoint(6,2),
		FNPrimitiveFontPoint(6,4),

		FNPrimitiveFontPoint(6,4),
		FNPrimitiveFontPoint(4,6),

		FNPrimitiveFontPoint(4,6),
		FNPrimitiveFontPoint(2,6),

		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(0,4),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(0,2)
	};
	
	// p
	Glyphs[112] = TArray {
		FNPrimitiveFontPoint(0,2),
		FNPrimitiveFontPoint(2,0),

		FNPrimitiveFontPoint(2,0),
		FNPrimitiveFontPoint(4,0),

		FNPrimitiveFontPoint(4,0),
		FNPrimitiveFontPoint(6,2),

		FNPrimitiveFontPoint(6,2),
		FNPrimitiveFontPoint(6,4),

		FNPrimitiveFontPoint(6,4),
		FNPrimitiveFontPoint(4,6),

		FNPrimitiveFontPoint(4,6),
		FNPrimitiveFontPoint(2,6),

		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(0,4),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(0,-2)
	};
	
	// q
	Glyphs[113] = TArray {
		FNPrimitiveFontPoint(0,2),
		FNPrimitiveFontPoint(2,0),

		FNPrimitiveFontPoint(2,0),
		FNPrimitiveFontPoint(4,0),

		FNPrimitiveFontPoint(4,0),
		FNPrimitiveFontPoint(6,2),

		FNPrimitiveFontPoint(6,4),
		FNPrimitiveFontPoint(4,6),

		FNPrimitiveFontPoint(4,6),
		FNPrimitiveFontPoint(2,6),

		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(0,4),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(0,2),

		FNPrimitiveFontPoint(6,-2),
		FNPrimitiveFontPoint(6,4)
	};
	
	// r
	Glyphs[114] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(0,4),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(2,6),

		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(4,6),

		FNPrimitiveFontPoint(4,6),
		FNPrimitiveFontPoint(6,4)
	};
	
	// s
	Glyphs[115] = TArray {
		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(3,0),

		FNPrimitiveFontPoint(3,0),
		FNPrimitiveFontPoint(5,1),

		FNPrimitiveFontPoint(5,1),
		FNPrimitiveFontPoint(6,2),

		FNPrimitiveFontPoint(6,2),
		FNPrimitiveFontPoint(6,3),
		
		FNPrimitiveFontPoint(6,3),
		FNPrimitiveFontPoint(5,4),

		FNPrimitiveFontPoint(5,4),
		FNPrimitiveFontPoint(0,4),

		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(0,5),

		FNPrimitiveFontPoint(0,5),
		FNPrimitiveFontPoint(2,6),

		FNPrimitiveFontPoint(2,6),
		FNPrimitiveFontPoint(5,6),

		FNPrimitiveFontPoint(5,6),
		FNPrimitiveFontPoint(6,5)
	};
	
	// t
	Glyphs[116] = TArray {
		FNPrimitiveFontPoint(3,0),
		FNPrimitiveFontPoint(3,8),

		FNPrimitiveFontPoint(1,4),
		FNPrimitiveFontPoint(5,4)
	};
	
	// u
	Glyphs[117] = TArray {
		FNPrimitiveFontPoint(0,6),
		FNPrimitiveFontPoint(0,2),

		FNPrimitiveFontPoint(0,2),
		FNPrimitiveFontPoint(2,0),

		FNPrimitiveFontPoint(2,0),
		FNPrimitiveFontPoint(4,0),

		FNPrimitiveFontPoint(4,0),
		FNPrimitiveFontPoint(6,2),

		FNPrimitiveFontPoint(6,2),
		FNPrimitiveFontPoint(6,6)
	};
	
	// v
	Glyphs[118] = TArray {
		FNPrimitiveFontPoint(0,6),
		FNPrimitiveFontPoint(3,0),

		FNPrimitiveFontPoint(3,0),
		FNPrimitiveFontPoint(6,6)
	};
	
	// w
	Glyphs[119] = TArray {
		FNPrimitiveFontPoint(0,6),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(2,0),

		FNPrimitiveFontPoint(2,0),
		FNPrimitiveFontPoint(3,2),
		
		FNPrimitiveFontPoint(3,2),
		FNPrimitiveFontPoint(4,0),

		FNPrimitiveFontPoint(4,0),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(6,0),
		FNPrimitiveFontPoint(6,6),
	};
	
	// x
	Glyphs[120] = TArray {
		FNPrimitiveFontPoint(0,6),
		FNPrimitiveFontPoint(6,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,6)
	};
	
	// y
	Glyphs[121] = TArray {
		FNPrimitiveFontPoint(0,6),
		FNPrimitiveFontPoint(0,2),

		FNPrimitiveFontPoint(0,2),
		FNPrimitiveFontPoint(2, 0),

		FNPrimitiveFontPoint(2, 0),
		FNPrimitiveFontPoint(4, 0),

		FNPrimitiveFontPoint(4, 0),
		FNPrimitiveFontPoint(6, 2),

		FNPrimitiveFontPoint(6, 6),
		FNPrimitiveFontPoint(6, -2),

		FNPrimitiveFontPoint(6, -2),
		FNPrimitiveFontPoint(3, -3),

		FNPrimitiveFontPoint(3, -3),
		FNPrimitiveFontPoint(0, -1)
	};
	
	// z
	Glyphs[122] = TArray {
		FNPrimitiveFontPoint(0,6),
		FNPrimitiveFontPoint(6,6),

		FNPrimitiveFontPoint(6,6),
		FNPrimitiveFontPoint(0,0),

		FNPrimitiveFontPoint(0,0),
		FNPrimitiveFontPoint(6,0)
	};

	// {
	Glyphs[123] = TArray {
		FNPrimitiveFontPoint(4,8),
		FNPrimitiveFontPoint(2,8),
		
		FNPrimitiveFontPoint(2,8),
		FNPrimitiveFontPoint(1,7),
		
		FNPrimitiveFontPoint(1,7),
		FNPrimitiveFontPoint(1,5),
		
		FNPrimitiveFontPoint(1,5),
		FNPrimitiveFontPoint(0,4),
		
		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(1,3),
		
		FNPrimitiveFontPoint(1,3),
		FNPrimitiveFontPoint(1,1),
		
		FNPrimitiveFontPoint(1,1),
		FNPrimitiveFontPoint(2,0),
		
		FNPrimitiveFontPoint(2,0),
		FNPrimitiveFontPoint(4,0)
	};

	// |
	Glyphs[124] = TArray {
		FNPrimitiveFontPoint(3,0),
		FNPrimitiveFontPoint(3,8)
	};

	// }
	Glyphs[125] = TArray {
		FNPrimitiveFontPoint(2,8),
		FNPrimitiveFontPoint(4,8),
		
		FNPrimitiveFontPoint(4,8),
		FNPrimitiveFontPoint(5,7),
		
		FNPrimitiveFontPoint(5,7),
		FNPrimitiveFontPoint(5,5),
		
		FNPrimitiveFontPoint(5,5),
		FNPrimitiveFontPoint(6,4),
		
		FNPrimitiveFontPoint(6,4),
		FNPrimitiveFontPoint(5,3),
		
		FNPrimitiveFontPoint(5,3),
		FNPrimitiveFontPoint(5,1),
		
		FNPrimitiveFontPoint(5,1),
		FNPrimitiveFontPoint(4,0),
		
		FNPrimitiveFontPoint(4,0),
		FNPrimitiveFontPoint(2,0)
	};

	// ~
	Glyphs[126] = TArray {
		FNPrimitiveFontPoint(0,3),
		FNPrimitiveFontPoint(0,4),
		
		FNPrimitiveFontPoint(0,4),
		FNPrimitiveFontPoint(1, 5),
		
		FNPrimitiveFontPoint(1, 5),
		FNPrimitiveFontPoint(2, 5),
		
		FNPrimitiveFontPoint(2, 5),
		FNPrimitiveFontPoint(4, 3),
		
		FNPrimitiveFontPoint(4, 3),
		FNPrimitiveFontPoint(5, 3),
		
		FNPrimitiveFontPoint(5, 3),
		FNPrimitiveFontPoint(6, 4),
		
		FNPrimitiveFontPoint(6, 4),
		FNPrimitiveFontPoint(6, 5)
	};
	
	// Flag that we have generated at this point.
	bIsInitialized = true;
}