// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Cell/NCellVoxelData.h"
#include "Misc/Timespan.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_CRITICAL(FNCellVoxelDataTests_Uniform, "NEXUS::UnitTests::NProcGen::FNCellVoxelData::Uniform", N_TEST_CONTEXT_ANYWHERE)
{
	FNCellVoxelData VoxelData;
	VoxelData.Resize(10, 10, 10);

	VoxelData.SetData(0, 0, 0, static_cast<uint8>(ENCellVoxel::Occupied));
	VoxelData.SetData(9, 9, 9, static_cast<uint8>(ENCellVoxel::Occupied));
	VoxelData.SetData(0, 4, 4, static_cast<uint8>(ENCellVoxel::Occupied));
	VoxelData.SetData(6, 6, 9, static_cast<uint8>(ENCellVoxel::Occupied));

	auto [MidX, MidY, MidZ] = VoxelData.GetInverseIndex(241);
	VoxelData.SetData(241, static_cast<uint8>(ENCellVoxel::Occupied));

	CHECK_EQUALS(TEXT("Expected 1000 flat array count."), VoxelData.GetCount(), static_cast<SIZE_T>(1000));
	CHECK_EQUALS(TEXT("Expected 9,9,9 to be occupied when read from flat array."), VoxelData.GetData(999), static_cast<uint8>(ENCellVoxel::Occupied))

	auto [InverseX, InverseY, InverseZ] = VoxelData.GetInverseIndex(999);
	CHECK_EQUALS(TEXT("InverseLastIndex:X expected 9"), InverseX, 9u)
	CHECK_EQUALS(TEXT("InverseLastIndex:Y expected 9"), InverseY, 9u)
	CHECK_EQUALS(TEXT("InverseLastIndex:Z expected 9"), InverseZ, 9u)

	const uint8 AlteredData = VoxelData.GetData(MidX, MidY, MidZ);
	CHECK_EQUALS(TEXT("Expected round tripped altered data to match."), AlteredData, static_cast<uint8>(ENCellVoxel::Occupied))
}

N_TEST_HIGH(FNCellVoxelDataTests_DefaultCellsAreEmpty, "NEXUS::UnitTests::NProcGen::FNCellVoxelData::DefaultCellsAreEmpty", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that every cell is Empty (0) after Resize without any explicit writes.
	FNCellVoxelData VoxelData;
	VoxelData.Resize(3, 4, 5);

	const SIZE_T Count = VoxelData.GetCount();
	CHECK_EQUALS("Non-cubic 3x4x5 grid should have 60 cells", Count, static_cast<SIZE_T>(60));

	for (uint32 i = 0; i < 60; ++i)
	{
		if (VoxelData.GetData(i) != static_cast<uint8>(ENCellVoxel::Empty))
		{
			ADD_ERROR(FString::Printf(TEXT("Cell at flat index %u should default to Empty"), i));
			return;
		}
	}
}

N_TEST_HIGH(FNCellVoxelDataTests_GetIndex_Asymmetric, "NEXUS::UnitTests::NProcGen::FNCellVoxelData::GetIndex_Asymmetric", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies the flat index formula on a non-cubic grid where every axis stride is distinct,
	// so an X/Y or X/Z swap in the implementation would produce a different value.
	FNCellVoxelData VoxelData;
	VoxelData.Resize(4, 5, 6);

	// Expected: 1 + 2*4 + 3*(4*5) = 1 + 8 + 60 = 69.
	CHECK_EQUALS("GetIndex(1,2,3) on 4x5x6 grid should equal 69", VoxelData.GetIndex(1u, 2u, 3u), 69u);
}

N_TEST_HIGH(FNCellVoxelDataTests_InverseIndex_Asymmetric, "NEXUS::UnitTests::NProcGen::FNCellVoxelData::InverseIndex_Asymmetric", N_TEST_CONTEXT_ANYWHERE)
{
	FNCellVoxelData VoxelData;
	VoxelData.Resize(4, 5, 6);

	auto [X, Y, Z] = VoxelData.GetInverseIndex(69u);
	CHECK_EQUALS("InverseIndex(69):X on 4x5x6 should be 1", X, 1u);
	CHECK_EQUALS("InverseIndex(69):Y on 4x5x6 should be 2", Y, 2u);
	CHECK_EQUALS("InverseIndex(69):Z on 4x5x6 should be 3", Z, 3u);
}

N_TEST_HIGH(FNCellVoxelDataTests_IndexRoundTrip_NonCubic, "NEXUS::UnitTests::NProcGen::FNCellVoxelData::IndexRoundTrip_NonCubic", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies GetIndex / GetInverseIndex are exact inverses for every cell on a non-cubic grid.
	FNCellVoxelData VoxelData;
	VoxelData.Resize(3, 4, 5);

	for (uint32 z = 0; z < 5; ++z)
	{
		for (uint32 y = 0; y < 4; ++y)
		{
			for (uint32 x = 0; x < 3; ++x)
			{
				const uint32 Flat = VoxelData.GetIndex(x, y, z);
				auto [Rx, Ry, Rz] = VoxelData.GetInverseIndex(Flat);
				if (Rx != x || Ry != y || Rz != z)
				{
					ADD_ERROR(FString::Printf(TEXT("Round trip failed for (%u,%u,%u) -> %u -> (%u,%u,%u)"), x, y, z, Flat, Rx, Ry, Rz));
					return;
				}
			}
		}
	}
}

N_TEST_HIGH(FNCellVoxelDataTests_SetGet_DistinctCells, "NEXUS::UnitTests::NProcGen::FNCellVoxelData::SetGet_DistinctCells", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that writes at distinct, non-symmetric cells do not bleed into one another.
	FNCellVoxelData VoxelData;
	VoxelData.Resize(5, 6, 7);
	VoxelData.SetData(1u, 2u, 3u, static_cast<uint8>(ENCellVoxel::Occupied));
	VoxelData.SetData(4u, 0u, 5u, static_cast<uint8>(ENCellVoxel::Occupied));

	CHECK_EQUALS("Cell (1,2,3) should be Occupied", VoxelData.GetData(1u, 2u, 3u), static_cast<uint8>(ENCellVoxel::Occupied));
	CHECK_EQUALS("Cell (4,0,5) should be Occupied", VoxelData.GetData(4u, 0u, 5u), static_cast<uint8>(ENCellVoxel::Occupied));
	CHECK_EQUALS("Unrelated cell (0,0,0) should remain Empty", VoxelData.GetData(0u, 0u, 0u), static_cast<uint8>(ENCellVoxel::Empty));
	CHECK_EQUALS("Unrelated cell (4,5,6) should remain Empty", VoxelData.GetData(4u, 5u, 6u), static_cast<uint8>(ENCellVoxel::Empty));
}

N_TEST_HIGH(FNCellVoxelDataTests_SetGet_NoAxisSwap, "NEXUS::UnitTests::NProcGen::FNCellVoxelData::SetGet_NoAxisSwap", N_TEST_CONTEXT_ANYWHERE)
{
	// Sets one asymmetric cell on a cubic grid and verifies that every axis permutation reads Empty.
	// Catches regressions where the implementation accidentally swaps two axes.
	FNCellVoxelData VoxelData;
	VoxelData.Resize(5, 5, 5);
	VoxelData.SetData(3u, 1u, 2u, static_cast<uint8>(ENCellVoxel::Occupied));

	CHECK_EQUALS("Original cell (3,1,2) should read Occupied", VoxelData.GetData(3u, 1u, 2u), static_cast<uint8>(ENCellVoxel::Occupied));
	CHECK_EQUALS("Permuted cell (1,3,2) should be Empty", VoxelData.GetData(1u, 3u, 2u), static_cast<uint8>(ENCellVoxel::Empty));
	CHECK_EQUALS("Permuted cell (2,1,3) should be Empty", VoxelData.GetData(2u, 1u, 3u), static_cast<uint8>(ENCellVoxel::Empty));
	CHECK_EQUALS("Permuted cell (3,2,1) should be Empty", VoxelData.GetData(3u, 2u, 1u), static_cast<uint8>(ENCellVoxel::Empty));
	CHECK_EQUALS("Permuted cell (1,2,3) should be Empty", VoxelData.GetData(1u, 2u, 3u), static_cast<uint8>(ENCellVoxel::Empty));
	CHECK_EQUALS("Permuted cell (2,3,1) should be Empty", VoxelData.GetData(2u, 3u, 1u), static_cast<uint8>(ENCellVoxel::Empty));
}

N_TEST_HIGH(FNCellVoxelDataTests_Resize_PreservesInRangeCells, "NEXUS::UnitTests::NProcGen::FNCellVoxelData::Resize_PreservesInRangeCells", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that growing the grid keeps existing in-range cell values intact.
	FNCellVoxelData VoxelData;
	VoxelData.Resize(3, 3, 3);
	VoxelData.SetData(0u, 0u, 0u, static_cast<uint8>(ENCellVoxel::Occupied));
	VoxelData.SetData(2u, 2u, 2u, static_cast<uint8>(ENCellVoxel::Occupied));

	VoxelData.Resize(5, 5, 5);

	CHECK_EQUALS("Cell count should reflect the new dimensions", VoxelData.GetCount(), static_cast<SIZE_T>(125));
	CHECK_EQUALS("Cell (0,0,0) should still be Occupied after grow", VoxelData.GetData(0u, 0u, 0u), static_cast<uint8>(ENCellVoxel::Occupied));
	CHECK_EQUALS("Cell (2,2,2) should still be Occupied after grow", VoxelData.GetData(2u, 2u, 2u), static_cast<uint8>(ENCellVoxel::Occupied));
	CHECK_EQUALS("Newly added cell (4,4,4) should default to Empty", VoxelData.GetData(4u, 4u, 4u), static_cast<uint8>(ENCellVoxel::Empty));
}

N_TEST_HIGH(FNCellVoxelDataTests_Resize_TruncatesShrunkCells, "NEXUS::UnitTests::NProcGen::FNCellVoxelData::Resize_TruncatesShrunkCells", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that shrinking the grid keeps in-range cells, drops the rest, and updates the count.
	FNCellVoxelData VoxelData;
	VoxelData.Resize(4, 4, 4);
	VoxelData.SetData(1u, 1u, 1u, static_cast<uint8>(ENCellVoxel::Occupied));
	VoxelData.SetData(3u, 3u, 3u, static_cast<uint8>(ENCellVoxel::Occupied));

	VoxelData.Resize(2, 2, 2);

	CHECK_EQUALS("Cell count should drop to 2x2x2 = 8", VoxelData.GetCount(), static_cast<SIZE_T>(8));
	CHECK_EQUALS("Cell (1,1,1) should still be Occupied after shrink", VoxelData.GetData(1u, 1u, 1u), static_cast<uint8>(ENCellVoxel::Occupied));
}

N_TEST_HIGH(FNCellVoxelDataTests_Reset_WipesData, "NEXUS::UnitTests::NProcGen::FNCellVoxelData::Reset_WipesData", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies Reset returns every cell to the Empty default.
	FNCellVoxelData VoxelData;
	VoxelData.Resize(3, 3, 3);
	VoxelData.SetData(0u, 0u, 0u, static_cast<uint8>(ENCellVoxel::Occupied));
	VoxelData.SetData(2u, 2u, 2u, static_cast<uint8>(ENCellVoxel::Occupied));

	VoxelData.Reset(3, 3, 3);

	CHECK_EQUALS("Reset should keep dimensions but clear contents", VoxelData.GetCount(), static_cast<SIZE_T>(27));
	for (uint32 i = 0; i < 27; ++i)
	{
		if (VoxelData.GetData(i) != static_cast<uint8>(ENCellVoxel::Empty))
		{
			ADD_ERROR(FString::Printf(TEXT("Cell at flat index %u should be Empty after Reset"), i));
			return;
		}
	}
}

N_TEST_HIGH(FNCellVoxelDataTests_FlatVsCoord_Equivalence, "NEXUS::UnitTests::NProcGen::FNCellVoxelData::FlatVsCoord_Equivalence", N_TEST_CONTEXT_ANYWHERE)
{
	// Verifies that writing via flat index and reading via (x,y,z) - and vice versa - produce
	// the same value at the same logical cell.
	FNCellVoxelData VoxelData;
	VoxelData.Resize(4, 5, 6);

	const uint32 Flat = VoxelData.GetIndex(2u, 3u, 4u);
	VoxelData.SetData(Flat, static_cast<uint8>(ENCellVoxel::Occupied));
	CHECK_EQUALS("Coord read should match flat write", VoxelData.GetData(2u, 3u, 4u), static_cast<uint8>(ENCellVoxel::Occupied));

	VoxelData.SetData(1u, 0u, 5u, static_cast<uint8>(ENCellVoxel::Occupied));
	const uint32 Flat2 = VoxelData.GetIndex(1u, 0u, 5u);
	CHECK_EQUALS("Flat read should match coord write", VoxelData.GetData(Flat2), static_cast<uint8>(ENCellVoxel::Occupied));
}

#endif //WITH_TESTS
