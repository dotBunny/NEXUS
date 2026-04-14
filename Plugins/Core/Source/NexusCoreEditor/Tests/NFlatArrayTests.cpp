// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Macros/NArrayMacros.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

// ─── Test fixtures using the flat-array macros ────────────────────────────────

struct FTestFlat2DArray
{
	int32 SizeX = 0;
	int32 SizeY = 0;
	TArray<int32> Data;

	N_IMPLEMENT_FLAT_2D_ARRAY(int32, int32, Data, SizeX, SizeY)
};

struct FTestFlat3DArray
{
	int32 SizeX = 0;
	int32 SizeY = 0;
	int32 SizeZ = 0;
	TArray<int32> Data;

	N_IMPLEMENT_FLAT_3D_ARRAY(int32, int32, Data, SizeX, SizeY, SizeZ)
};

// ─── 2D Array — Reset and element count ───────────────────────────────────────

N_TEST_HIGH(FNFlatArray2D_Reset_SetsCorrectCount, "NEXUS::UnitTests::NCore::FlatArray2D::Reset_SetsCorrectCount", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat2DArray Grid;
	Grid.Reset(4, 3);
	CHECK_MESSAGE(TEXT("Reset(4,3) should produce 12 elements"), Grid.GetCount() == 12);
}

N_TEST_HIGH(FNFlatArray2D_Reset_ZeroInitializes, "NEXUS::UnitTests::NCore::FlatArray2D::Reset_ZeroInitializes", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat2DArray Grid;
	Grid.Reset(3, 3);
	for (int32 i = 0; i < 9; ++i)
	{
		CHECK_MESSAGE(FString::Printf(TEXT("Data[%d] should be zero after Reset"), i), Grid.GetData(i) == 0);
	}
}

// ─── 2D Array — GetIndex / GetInverseIndex round-trip ─────────────────────────

N_TEST_HIGH(FNFlatArray2D_GetIndex_LinearMapping, "NEXUS::UnitTests::NCore::FlatArray2D::GetIndex_LinearMapping", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat2DArray Grid;
	Grid.Reset(5, 4);

	// Index formula: X + Y * SizeX
	CHECK_MESSAGE(TEXT("(0,0) should map to index 0"), Grid.GetIndex(0, 0) == 0);
	CHECK_MESSAGE(TEXT("(1,0) should map to index 1"), Grid.GetIndex(1, 0) == 1);
	CHECK_MESSAGE(TEXT("(0,1) should map to index 5"), Grid.GetIndex(0, 1) == 5);
	CHECK_MESSAGE(TEXT("(4,3) should map to last index 19"), Grid.GetIndex(4, 3) == 19);
}

N_TEST_HIGH(FNFlatArray2D_GetInverseIndex_RoundTrip, "NEXUS::UnitTests::NCore::FlatArray2D::GetInverseIndex_RoundTrip", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat2DArray Grid;
	Grid.Reset(5, 4);

	for (int32 y = 0; y < 4; ++y)
	{
		for (int32 x = 0; x < 5; ++x)
		{
			const int32 Index = Grid.GetIndex(x, y);
			const auto [IX, IY] = Grid.GetInverseIndex(Index);
			CHECK_MESSAGE(FString::Printf(TEXT("InverseIndex X should round-trip for (%d,%d)"), x, y), IX == x);
			CHECK_MESSAGE(FString::Printf(TEXT("InverseIndex Y should round-trip for (%d,%d)"), x, y), IY == y);
		}
	}
}

// ─── 2D Array — FVector2D and FIntVector2 index variants ──────────────────────

N_TEST_HIGH(FNFlatArray2D_GetIndex_FVector2D, "NEXUS::UnitTests::NCore::FlatArray2D::GetIndex_FVector2D", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat2DArray Grid;
	Grid.Reset(5, 4);
	// (2.9, 1.0) should floor to (2, 1) → 2 + 1*5 = 7
	CHECK_MESSAGE(TEXT("FVector2D index should floor to int coords"), Grid.GetIndex(FVector2D(2.9, 1.0)) == 7);
}

N_TEST_HIGH(FNFlatArray2D_GetIndex_FIntVector2, "NEXUS::UnitTests::NCore::FlatArray2D::GetIndex_FIntVector2", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat2DArray Grid;
	Grid.Reset(5, 4);
	// (3, 2) → 3 + 2*5 = 13
	CHECK_MESSAGE(TEXT("FIntVector2 index should match XY coords"), Grid.GetIndex(FIntVector2(3, 2)) == 13);
}

// ─── 2D Array — SetData / GetData ─────────────────────────────────────────────

N_TEST_HIGH(FNFlatArray2D_SetGetData, "NEXUS::UnitTests::NCore::FlatArray2D::SetGetData", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat2DArray Grid;
	Grid.Reset(4, 4);

	Grid.SetData(2, 3, 99);
	CHECK_MESSAGE(TEXT("SetData/GetData by XY should round-trip"), Grid.GetData(2, 3) == 99);

	const int32 LinearIndex = Grid.GetIndex(2, 3);
	CHECK_MESSAGE(TEXT("GetData by linear index should return same value"), Grid.GetData(LinearIndex) == 99);
}

// ─── 2D Array — GetSurroundingIndices ─────────────────────────────────────────

N_TEST_HIGH(FNFlatArray2D_GetSurroundingIndices_Center, "NEXUS::UnitTests::NCore::FlatArray2D::GetSurroundingIndices_Center", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat2DArray Grid;
	Grid.Reset(3, 3);

	TArray<int32> Neighbors;
	Grid.GetSurroundingIndices(1, 1, Neighbors);  // center of 3x3
	CHECK_MESSAGE(TEXT("Center cell in 3x3 grid should have 8 neighbors"), Neighbors.Num() == 8);
}

N_TEST_HIGH(FNFlatArray2D_GetSurroundingIndices_Corner, "NEXUS::UnitTests::NCore::FlatArray2D::GetSurroundingIndices_Corner", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat2DArray Grid;
	Grid.Reset(3, 3);

	TArray<int32> Neighbors;
	Grid.GetSurroundingIndices(0, 0, Neighbors);  // top-left corner
	CHECK_MESSAGE(TEXT("Corner cell in 3x3 grid should have 3 neighbors"), Neighbors.Num() == 3);
}

N_TEST_HIGH(FNFlatArray2D_GetSurroundingIndices_Edge, "NEXUS::UnitTests::NCore::FlatArray2D::GetSurroundingIndices_Edge", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat2DArray Grid;
	Grid.Reset(3, 3);

	TArray<int32> Neighbors;
	Grid.GetSurroundingIndices(1, 0, Neighbors);  // middle of bottom edge
	CHECK_MESSAGE(TEXT("Edge cell (non-corner) in 3x3 grid should have 5 neighbors"), Neighbors.Num() == 5);
}

// ─── 3D Array — Reset and element count ───────────────────────────────────────

N_TEST_HIGH(FNFlatArray3D_Reset_SetsCorrectCount, "NEXUS::UnitTests::NCore::FlatArray3D::Reset_SetsCorrectCount", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat3DArray Grid;
	Grid.Reset(2, 3, 4);
	CHECK_MESSAGE(TEXT("Reset(2,3,4) should produce 24 elements"), Grid.GetCount() == 24);
}

// ─── 3D Array — GetIndex / GetInverseIndex round-trip ─────────────────────────

N_TEST_HIGH(FNFlatArray3D_GetIndex_LinearMapping, "NEXUS::UnitTests::NCore::FlatArray3D::GetIndex_LinearMapping", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat3DArray Grid;
	Grid.Reset(4, 3, 2);

	// Index formula: X + Y*SizeX + Z*SizeX*SizeY
	CHECK_MESSAGE(TEXT("(0,0,0) should map to index 0"), Grid.GetIndex(0, 0, 0) == 0);
	CHECK_MESSAGE(TEXT("(1,0,0) should map to index 1"), Grid.GetIndex(1, 0, 0) == 1);
	CHECK_MESSAGE(TEXT("(0,1,0) should map to index 4"), Grid.GetIndex(0, 1, 0) == 4);
	CHECK_MESSAGE(TEXT("(0,0,1) should map to index 12"), Grid.GetIndex(0, 0, 1) == 12);
	CHECK_MESSAGE(TEXT("(3,2,1) should map to last index 23"), Grid.GetIndex(3, 2, 1) == 23);
}

N_TEST_HIGH(FNFlatArray3D_GetInverseIndex_RoundTrip, "NEXUS::UnitTests::NCore::FlatArray3D::GetInverseIndex_RoundTrip", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat3DArray Grid;
	Grid.Reset(4, 3, 2);

	for (int32 z = 0; z < 2; ++z)
	{
		for (int32 y = 0; y < 3; ++y)
		{
			for (int32 x = 0; x < 4; ++x)
			{
				const int32 Index = Grid.GetIndex(x, y, z);
				const auto [IX, IY, IZ] = Grid.GetInverseIndex(Index);
				CHECK_MESSAGE(FString::Printf(TEXT("InverseIndex X round-trip failed at (%d,%d,%d)"), x, y, z), IX == x);
				CHECK_MESSAGE(FString::Printf(TEXT("InverseIndex Y round-trip failed at (%d,%d,%d)"), x, y, z), IY == y);
				CHECK_MESSAGE(FString::Printf(TEXT("InverseIndex Z round-trip failed at (%d,%d,%d)"), x, y, z), IZ == z);
			}
		}
	}
}

// ─── 3D Array — SetData / GetData ─────────────────────────────────────────────

N_TEST_HIGH(FNFlatArray3D_SetGetData, "NEXUS::UnitTests::NCore::FlatArray3D::SetGetData", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat3DArray Grid;
	Grid.Reset(4, 3, 2);

	Grid.SetData(2, 1, 1, 42);
	CHECK_MESSAGE(TEXT("SetData/GetData by XYZ should round-trip"), Grid.GetData(2, 1, 1) == 42);

	const int32 LinearIndex = Grid.GetIndex(2, 1, 1);
	CHECK_MESSAGE(TEXT("GetData by linear index should return the stored value"), Grid.GetData(LinearIndex) == 42);
}

// ─── 2D Array — Resize ────────────────────────────────────────────────────────

N_TEST_HIGH(FNFlatArray2D_Resize_GrowPreservesData, "NEXUS::UnitTests::NCore::FlatArray2D::Resize_GrowPreservesData", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat2DArray Grid;
	Grid.Reset(3, 3);

	// Write a known value at every cell
	for (int32 y = 0; y < 3; ++y)
	{
		for (int32 x = 0; x < 3; ++x)
		{
			Grid.SetData(x, y, x + y * 3);
		}
	}

	Grid.Resize(5, 5);

	CHECK_MESSAGE(TEXT("Resize grow should produce 25 elements"), Grid.GetCount() == 25);

	// All cells that existed before the resize should retain their values
	for (int32 y = 0; y < 3; ++y)
	{
		for (int32 x = 0; x < 3; ++x)
		{
			CHECK_MESSAGE(FString::Printf(TEXT("Resize grow should preserve data at (%d,%d)"), x, y), Grid.GetData(x, y) == x + y * 3);
		}
	}
}

N_TEST_HIGH(FNFlatArray2D_Resize_ShrinkPreservesData, "NEXUS::UnitTests::NCore::FlatArray2D::Resize_ShrinkPreservesData", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat2DArray Grid;
	Grid.Reset(4, 4);

	for (int32 y = 0; y < 4; ++y)
	{
		for (int32 x = 0; x < 4; ++x)
		{
			Grid.SetData(x, y, x + y * 4);
		}
	}

	Grid.Resize(2, 2);

	CHECK_MESSAGE(TEXT("Resize shrink should produce 4 elements"), Grid.GetCount() == 4);

	// The surviving 2x2 region should retain their original values
	for (int32 y = 0; y < 2; ++y)
	{
		for (int32 x = 0; x < 2; ++x)
		{
			CHECK_MESSAGE(FString::Printf(TEXT("Resize shrink should preserve data at (%d,%d)"), x, y), Grid.GetData(x, y) == x + y * 4);
		}
	}
}

N_TEST_HIGH(FNFlatArray2D_Resize_AsymmetricGrow, "NEXUS::UnitTests::NCore::FlatArray2D::Resize_AsymmetricGrow", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat2DArray Grid;
	Grid.Reset(2, 2);
	Grid.SetData(0, 0, 10);
	Grid.SetData(1, 0, 20);
	Grid.SetData(0, 1, 30);
	Grid.SetData(1, 1, 40);

	// Grow only in X
	Grid.Resize(4, 2);

	CHECK_MESSAGE(TEXT("Asymmetric grow should produce 8 elements"), Grid.GetCount() == 8);
	CHECK_MESSAGE(TEXT("(0,0) preserved after asymmetric grow"), Grid.GetData(0, 0) == 10);
	CHECK_MESSAGE(TEXT("(1,0) preserved after asymmetric grow"), Grid.GetData(1, 0) == 20);
	CHECK_MESSAGE(TEXT("(0,1) preserved after asymmetric grow"), Grid.GetData(0, 1) == 30);
	CHECK_MESSAGE(TEXT("(1,1) preserved after asymmetric grow"), Grid.GetData(1, 1) == 40);
}

// ─── 3D Array — Resize ────────────────────────────────────────────────────────

N_TEST_HIGH(FNFlatArray3D_Resize_GrowPreservesData, "NEXUS::UnitTests::NCore::FlatArray3D::Resize_GrowPreservesData", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat3DArray Grid;
	Grid.Reset(2, 2, 2);

	for (int32 z = 0; z < 2; ++z)
	{
		for (int32 y = 0; y < 2; ++y)
		{
			for (int32 x = 0; x < 2; ++x)
			{
				Grid.SetData(x, y, z, x + y * 2 + z * 4);
			}
		}
	}

	Grid.Resize(3, 3, 3);

	CHECK_MESSAGE(TEXT("Resize grow should produce 27 elements"), Grid.GetCount() == 27);

	// All cells that existed before the resize should retain their values
	for (int32 z = 0; z < 2; ++z)
	{
		for (int32 y = 0; y < 2; ++y)
		{
			for (int32 x = 0; x < 2; ++x)
			{
				CHECK_MESSAGE(FString::Printf(TEXT("Resize grow should preserve data at (%d,%d,%d)"), x, y, z), Grid.GetData(x, y, z) == x + y * 2 + z * 4);
			}
		}
	}
}

N_TEST_HIGH(FNFlatArray3D_Resize_ShrinkPreservesData, "NEXUS::UnitTests::NCore::FlatArray3D::Resize_ShrinkPreservesData", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat3DArray Grid;
	Grid.Reset(3, 3, 3);

	for (int32 z = 0; z < 3; ++z)
	{
		for (int32 y = 0; y < 3; ++y)
		{
			for (int32 x = 0; x < 3; ++x)
			{
				Grid.SetData(x, y, z, x + y * 3 + z * 9);
			}
		}
	}

	Grid.Resize(2, 2, 2);

	CHECK_MESSAGE(TEXT("Resize shrink should produce 8 elements"), Grid.GetCount() == 8);

	for (int32 z = 0; z < 2; ++z)
	{
		for (int32 y = 0; y < 2; ++y)
		{
			for (int32 x = 0; x < 2; ++x)
			{
				CHECK_MESSAGE(FString::Printf(TEXT("Resize shrink should preserve data at (%d,%d,%d)"), x, y, z), Grid.GetData(x, y, z) == x + y * 3 + z * 9);
			}
		}
	}
}

// ─── 3D Array — GetSurroundingIndices ─────────────────────────────────────────

N_TEST_HIGH(FNFlatArray3D_GetSurroundingIndices_Center, "NEXUS::UnitTests::NCore::FlatArray3D::GetSurroundingIndices_Center", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat3DArray Grid;
	Grid.Reset(3, 3, 3);

	TArray<int32> Neighbors;
	Grid.GetSurroundingIndices(1, 1, 1, Neighbors);  // center of 3x3x3
	// A fully-surrounded interior cell in a 3D grid has 26 neighbors
	CHECK_MESSAGE(TEXT("Center cell in 3x3x3 grid should have 26 neighbors"), Neighbors.Num() == 26);
}

N_TEST_HIGH(FNFlatArray3D_GetSurroundingIndices_Corner, "NEXUS::UnitTests::NCore::FlatArray3D::GetSurroundingIndices_Corner", N_TEST_CONTEXT_ANYWHERE)
{
	FTestFlat3DArray Grid;
	Grid.Reset(3, 3, 3);

	TArray<int32> Neighbors;
	Grid.GetSurroundingIndices(0, 0, 0, Neighbors);  // corner of 3x3x3
	// A corner cell has 7 neighbors (3 face, 3 edge, 1 corner)
	CHECK_MESSAGE(TEXT("Corner cell in 3x3x3 grid should have 7 neighbors"), Neighbors.Num() == 7);
}

#endif //WITH_TESTS