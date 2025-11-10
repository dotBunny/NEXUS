// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Cell/NCellVoxelData.h"
#include "Misc/Timespan.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_CRITICAL(FNProcGenTests_CellVoxelData_Uniform, "NEXUS::UnitTests::NProcGen::NCellVoxelData_Uniform", N_TEST_CONTEXT_ANYWHERE)
{
	FNCellVoxelData VoxelData;
	
	VoxelData.Resize(10, 10, 10);

	// Establish expected Data
	VoxelData.SetData(0, 0, 0, static_cast<uint8>(ENCellVoxel::CVD_Occupied));
	VoxelData.SetData(9, 9, 9, static_cast<uint8>(ENCellVoxel::CVD_Occupied));
	VoxelData.SetData(0, 4, 4, static_cast<uint8>(ENCellVoxel::CVD_Occupied));
	VoxelData.SetData(6, 6, 9, static_cast<uint8>(ENCellVoxel::CVD_Occupied));
	
	auto MidIndex = VoxelData.GetInverseIndex(241);
	auto [MidX, MidY, MidZ] = MidIndex;
	VoxelData.SetData(241,  static_cast<uint8>(ENCellVoxel::CVD_Occupied));
	
	
	// TEST
	{
		CHECK_EQUALS(TEXT("Expected 1000 flat array count."), VoxelData.GetCount(), static_cast<SIZE_T>(1000));
		CHECK_EQUALS(TEXT("Expected 9,9,9 to be occupied when read from flat array."), VoxelData.GetData(999),  static_cast<uint8>(ENCellVoxel::CVD_Occupied))
		
		auto InverseLastIndex = VoxelData.GetInverseIndex(999);
		auto [InverseX, InverseY, InverseZ] = InverseLastIndex;
		CHECK_EQUALS(TEXT("InverseLastIndex:X expected 9"), InverseX, 9)
		CHECK_EQUALS(TEXT("InverseLastIndex:Y expected 9"), InverseY, 9)
		CHECK_EQUALS(TEXT("InverseLastIndex:Z expected 9"), InverseZ, 9)
		
		const uint8 AlteredData = VoxelData.GetData(MidX, MidY, MidZ);
		CHECK_EQUALS(TEXT("Expected round tripped altered data to match."), AlteredData, static_cast<uint8>(ENCellVoxel::CVD_Occupied))
	}
}

#endif //WITH_TESTS
