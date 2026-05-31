// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NVoxelUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

N_TEST_LOW(FNVoxelCoordinateTests_DefaultConstruction_AllAxesZero,
	"NEXUS::UnitTests::NCore::FNVoxelCoordinate::DefaultConstruction::AllAxesZero",
	N_TEST_CONTEXT_ANYWHERE)
{
	// FNVoxelCoordinate has no methods — only X/Y/Z uint32 members. The only meaningful contract is that
	// default construction yields (0, 0, 0) so downstream consumers can rely on the origin voxel without
	// explicit initialization.
	const FNVoxelCoordinate Coord;
	CHECK_EQUALS("Default-constructed FNVoxelCoordinate.X should be 0", static_cast<int32>(Coord.X), 0);
	CHECK_EQUALS("Default-constructed FNVoxelCoordinate.Y should be 0", static_cast<int32>(Coord.Y), 0);
	CHECK_EQUALS("Default-constructed FNVoxelCoordinate.Z should be 0", static_cast<int32>(Coord.Z), 0);
}

#endif //WITH_TESTS
