// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NVectorUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(FNVectorUtilsTests_TransformPoint_NoRotation, "NEXUS::UnitTests::NCore::FNVectorUtils::TransformPoint_NoRotation", N_TEST_CONTEXT_ANYWHERE)
{
	// With zero rotation the result should be Origin + Point
	const FVector Result = FNVectorUtils::TransformPoint(FVector(10.f, 0.f, 0.f), FVector(5.f, 5.f, 5.f), FRotator::ZeroRotator);
	CHECK_MESSAGE(TEXT("TransformPoint with no rotation should be Origin + Point"), Result.Equals(FVector(15.f, 5.f, 5.f)));
}

N_TEST_HIGH(FNVectorUtilsTests_TransformPoint_ZeroOrigin, "NEXUS::UnitTests::NCore::FNVectorUtils::TransformPoint_ZeroOrigin", N_TEST_CONTEXT_ANYWHERE)
{
	// Zero origin and no rotation: result should be Point unchanged
	const FVector Result = FNVectorUtils::TransformPoint(FVector(7.f, 3.f, 2.f), FVector::ZeroVector, FRotator::ZeroRotator);
	CHECK_MESSAGE(TEXT("TransformPoint with zero origin and no rotation should equal Point"), Result.Equals(FVector(7.f, 3.f, 2.f)));
}

N_TEST_HIGH(FNVectorUtilsTests_RotateAndOffsetPoint_NoRotation, "NEXUS::UnitTests::NCore::FNVectorUtils::RotateAndOffsetPoint_NoRotation", N_TEST_CONTEXT_ANYWHERE)
{
	// Zero rotation: Rotation.RotateVector(Point) == Point, so result should be Point + Offset
	const FVector Result = FNVectorUtils::RotateAndOffsetPoint(FVector(10.f, 0.f, 0.f), FRotator::ZeroRotator, FVector(5.f, 5.f, 5.f));
	CHECK_MESSAGE(TEXT("RotateAndOffsetPoint with no rotation should be Point + Offset"), Result.Equals(FVector(15.f, 5.f, 5.f)));
}

N_TEST_HIGH(FNVectorUtilsTests_RotateAndOffsetPoint_ZeroOffset, "NEXUS::UnitTests::NCore::FNVectorUtils::RotateAndOffsetPoint_ZeroOffset", N_TEST_CONTEXT_ANYWHERE)
{
	// Zero offset and no rotation: result should be Point unchanged
	const FVector Result = FNVectorUtils::RotateAndOffsetPoint(FVector(4.f, 5.f, 6.f), FRotator::ZeroRotator, FVector::ZeroVector);
	CHECK_MESSAGE(TEXT("RotateAndOffsetPoint with no rotation and zero offset should equal Point"), Result.Equals(FVector(4.f, 5.f, 6.f)));
}

N_TEST_HIGH(FNVectorUtilsTests_RotatedAroundPivot_ZeroRotation, "NEXUS::UnitTests::NCore::FNVectorUtils::RotatedAroundPivot_ZeroRotation", N_TEST_CONTEXT_ANYWHERE)
{
	// No rotation: vector should be unchanged regardless of pivot
	const FVector Pivot = FVector(5.f, 0.f, 0.f);
	const FVector Point = FVector(10.f, 0.f, 0.f);
	const FVector Result = FNVectorUtils::RotatedAroundPivot(Point, Pivot, FRotator::ZeroRotator);
	CHECK_MESSAGE(TEXT("RotatedAroundPivot with zero rotation should return the original vector"), Result.Equals(Point));
}

N_TEST_HIGH(FNVectorUtilsTests_RotatedAroundPivot_Yaw180, "NEXUS::UnitTests::NCore::FNVectorUtils::RotatedAroundPivot_Yaw180", N_TEST_CONTEXT_ANYWHERE)
{
	// 180-degree yaw rotation around origin: (10,0,0) → (-10,0,0)
	const FVector Pivot = FVector::ZeroVector;
	const FVector Point = FVector(10.f, 0.f, 0.f);
	const FRotator Rot180 = FRotator(0.f, 180.f, 0.f);
	const FVector Result = FNVectorUtils::RotatedAroundPivot(Point, Pivot, Rot180);
	CHECK_MESSAGE(TEXT("RotatedAroundPivot 180-degree yaw should negate X"), Result.Equals(FVector(-10.f, 0.f, 0.f), 0.01f));
}

N_TEST_HIGH(FNVectorUtilsTests_GetClosestGridIntersection_OnGrid, "NEXUS::UnitTests::NCore::FNVectorUtils::GetClosestGridIntersection_OnGrid", N_TEST_CONTEXT_ANYWHERE)
{
	// Point already on a grid intersection should return itself
	const FVector GridSize(50.f, 50.f, 50.f);
	const FVector Result = FNVectorUtils::GetClosestGridIntersection(FVector(100.f, 150.f, 200.f), GridSize);
	CHECK_MESSAGE(TEXT("Point on grid should snap to itself"), Result.Equals(FVector(100.f, 150.f, 200.f)));
}

N_TEST_HIGH(FNVectorUtilsTests_GetClosestGridIntersection_RoundsDown, "NEXUS::UnitTests::NCore::FNVectorUtils::GetClosestGridIntersection_RoundsDown", N_TEST_CONTEXT_ANYWHERE)
{
	// 74 is closer to 50 than to 100 with a grid size of 50
	const FVector GridSize(50.f, 50.f, 50.f);
	const FVector Result = FNVectorUtils::GetClosestGridIntersection(FVector(74.f, 74.f, 74.f), GridSize);
	CHECK_MESSAGE(TEXT("Point below midpoint should snap to lower grid intersection"), Result.Equals(FVector(50.f, 50.f, 50.f)));
}

N_TEST_HIGH(FNVectorUtilsTests_GetClosestGridIntersection_RoundsUp, "NEXUS::UnitTests::NCore::FNVectorUtils::GetClosestGridIntersection_RoundsUp", N_TEST_CONTEXT_ANYWHERE)
{
	// 76 is closer to 100 than to 50 with a grid size of 50
	const FVector GridSize(50.f, 50.f, 50.f);
	const FVector Result = FNVectorUtils::GetClosestGridIntersection(FVector(76.f, 76.f, 76.f), GridSize);
	CHECK_MESSAGE(TEXT("Point above midpoint should snap to upper grid intersection"), Result.Equals(FVector(100.f, 100.f, 100.f)));
}

N_TEST_HIGH(FNVectorUtilsTests_GetClosestGridIntersection_Zero, "NEXUS::UnitTests::NCore::FNVectorUtils::GetClosestGridIntersection_Zero", N_TEST_CONTEXT_ANYWHERE)
{
	const FVector GridSize(50.f, 50.f, 50.f);
	const FVector Result = FNVectorUtils::GetClosestGridIntersection(FVector::ZeroVector, GridSize);
	CHECK_MESSAGE(TEXT("Zero vector should snap to zero"), Result.Equals(FVector::ZeroVector));
}

N_TEST_HIGH(FNVectorUtilsTests_GetCrunchedGridUnit_OnGrid, "NEXUS::UnitTests::NCore::FNVectorUtils::GetCrunchedGridUnit_OnGrid", N_TEST_CONTEXT_ANYWHERE)
{
	// 100 / 50 = 2 exactly on grid
	CHECK_MESSAGE(TEXT("Value exactly on grid should return exact grid unit"), FNVectorUtils::GetCrunchedGridUnit(100.0, 50.0) == 2);
}

N_TEST_HIGH(FNVectorUtilsTests_GetCrunchedGridUnit_OffGrid, "NEXUS::UnitTests::NCore::FNVectorUtils::GetCrunchedGridUnit_OffGrid", N_TEST_CONTEXT_ANYWHERE)
{
	// 75 / 50 = 1.5 → CeilToInt(1.5) = 2
	CHECK_MESSAGE(TEXT("Value between grid units should ceil to next grid unit"), FNVectorUtils::GetCrunchedGridUnit(75.0, 50.0) == 2);
}

N_TEST_HIGH(FNVectorUtilsTests_GetCrunchedGridUnit_Zero, "NEXUS::UnitTests::NCore::FNVectorUtils::GetCrunchedGridUnit_Zero", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("Zero value should return zero grid unit"), FNVectorUtils::GetCrunchedGridUnit(0.0, 50.0) == 0);
}

#endif //WITH_TESTS
