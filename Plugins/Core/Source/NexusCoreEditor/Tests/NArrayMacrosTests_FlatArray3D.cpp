// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Macros/NArrayMacros.h"
#include "Macros/NTestMacros.h"

namespace NEXUS::UnitTests::NCore::NArrayMacros
{
	struct FTestFlat3DArray
	{
		int32 SizeX = 0;
		int32 SizeY = 0;
		int32 SizeZ = 0;
		TArray<int32> Data;

		N_IMPLEMENT_FLAT_3D_ARRAY(int32, int32, Data, SizeX, SizeY, SizeZ)
	};
}


N_TEST_HIGH(FNArrayMacrosTests_FlatArray3D_Reset_SetsCorrectCount, "NEXUS::UnitTests::NCore::NArrayMacros::FlatArray3D::Reset_SetsCorrectCount", N_TEST_CONTEXT_ANYWHERE)
{
	NEXUS::UnitTests::NCore::NArrayMacros::FTestFlat3DArray Grid;
	Grid.Reset(2, 3, 4);
	CHECK_MESSAGE(TEXT("Reset(2,3,4) should produce 24 elements"), Grid.GetCount() == 24);
}

N_TEST_HIGH(FNArrayMacrosTests_FlatArray3D_GetIndex_LinearMapping, "NEXUS::UnitTests::NCore::NArrayMacros::FlatArray3D::GetIndex_LinearMapping", N_TEST_CONTEXT_ANYWHERE)
{
	NEXUS::UnitTests::NCore::NArrayMacros::FTestFlat3DArray Grid;
	Grid.Reset(4, 3, 2);

	// Index formula: X + Y*SizeX + Z*SizeX*SizeY
	CHECK_MESSAGE(TEXT("(0,0,0) should map to index 0"), Grid.GetIndex(0, 0, 0) == 0);
	CHECK_MESSAGE(TEXT("(1,0,0) should map to index 1"), Grid.GetIndex(1, 0, 0) == 1);
	CHECK_MESSAGE(TEXT("(0,1,0) should map to index 4"), Grid.GetIndex(0, 1, 0) == 4);
	CHECK_MESSAGE(TEXT("(0,0,1) should map to index 12"), Grid.GetIndex(0, 0, 1) == 12);
	CHECK_MESSAGE(TEXT("(3,2,1) should map to last index 23"), Grid.GetIndex(3, 2, 1) == 23);
}

N_TEST_HIGH(FNArrayMacrosTests_FlatArray3D_GetInverseIndex_RoundTrip, "NEXUS::UnitTests::NCore::NArrayMacros::FlatArray3D::GetInverseIndex_RoundTrip", N_TEST_CONTEXT_ANYWHERE)
{
	NEXUS::UnitTests::NCore::NArrayMacros::FTestFlat3DArray Grid;
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

N_TEST_HIGH(FNArrayMacrosTests_FlatArray3D_SetGetData, "NEXUS::UnitTests::NCore::NArrayMacros::FlatArray3D::SetGetData", N_TEST_CONTEXT_ANYWHERE)
{
	NEXUS::UnitTests::NCore::NArrayMacros::FTestFlat3DArray Grid;
	Grid.Reset(4, 3, 2);

	Grid.SetData(2, 1, 1, 42);
	CHECK_MESSAGE(TEXT("SetData/GetData by XYZ should round-trip"), Grid.GetData(2, 1, 1) == 42);

	const int32 LinearIndex = Grid.GetIndex(2, 1, 1);
	CHECK_MESSAGE(TEXT("GetData by linear index should return the stored value"), Grid.GetData(LinearIndex) == 42);
}

N_TEST_HIGH(FNArrayMacrosTests_FlatArray3D_Resize_GrowPreservesData, "NEXUS::UnitTests::NCore::NArrayMacros::FlatArray3D::Resize_GrowPreservesData", N_TEST_CONTEXT_ANYWHERE)
{
	NEXUS::UnitTests::NCore::NArrayMacros::FTestFlat3DArray Grid;
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

N_TEST_HIGH(FNArrayMacrosTests_FlatArray3D_Resize_ShrinkPreservesData, "NEXUS::UnitTests::NCore::NArrayMacros::FlatArray3D::Resize_ShrinkPreservesData", N_TEST_CONTEXT_ANYWHERE)
{
	NEXUS::UnitTests::NCore::NArrayMacros::FTestFlat3DArray Grid;
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

N_TEST_HIGH(FNArrayMacrosTests_FlatArray3D_GetSurroundingIndices_Center, "NEXUS::UnitTests::NCore::NArrayMacros::FlatArray3D::GetSurroundingIndices_Center", N_TEST_CONTEXT_ANYWHERE)
{
	NEXUS::UnitTests::NCore::NArrayMacros::FTestFlat3DArray Grid;
	Grid.Reset(3, 3, 3);

	TArray<int32> Neighbors;
	Grid.GetSurroundingIndices(1, 1, 1, Neighbors);  // center of 3x3x3
	// A fully-surrounded interior cell in a 3D grid has 26 neighbors
	CHECK_MESSAGE(TEXT("Center cell in 3x3x3 grid should have 26 neighbors"), Neighbors.Num() == 26);
}

N_TEST_HIGH(FNArrayMacrosTests_FlatArray3D_GetSurroundingIndices_Corner, "NEXUS::UnitTests::NCore::NArrayMacros::FlatArray3D::GetSurroundingIndices_Corner", N_TEST_CONTEXT_ANYWHERE)
{
	NEXUS::UnitTests::NCore::NArrayMacros::FTestFlat3DArray Grid;
	Grid.Reset(3, 3, 3);

	TArray<int32> Neighbors;
	Grid.GetSurroundingIndices(0, 0, 0, Neighbors);  // corner of 3x3x3
	// A corner cell has 7 neighbors (3 face, 3 edge, 1 corner)
	CHECK_MESSAGE(TEXT("Corner cell in 3x3x3 grid should have 7 neighbors"), Neighbors.Num() == 7);
}

#endif //WITH_TESTS