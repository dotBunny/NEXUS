// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NVectorUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(FNVectorUtilsTests_TransformPoint_Yaw90WithOrigin, "NEXUS::UnitTests::NCore::FNVectorUtils::TransformPoint_Yaw90WithOrigin", N_TEST_CONTEXT_ANYWHERE)
{
	// Yaw90 rotates (10,0,0) -> (0,10,0); Origin (5,5,5) shifts the result to (5,15,5).
	const FVector Result = FNVectorUtils::TransformPoint(FVector(10.f, 0.f, 0.f), FVector(5.f, 5.f, 5.f), FRotator(0.f, 90.f, 0.f));
	CHECK_MESSAGE(TEXT("TransformPoint should rotate the point first, then translate by Origin"), Result.Equals(FVector(5.f, 15.f, 5.f), 0.01f));
}

N_TEST_HIGH(FNVectorUtilsTests_TransformPoint_Yaw180WithOrigin, "NEXUS::UnitTests::NCore::FNVectorUtils::TransformPoint_Yaw180WithOrigin", N_TEST_CONTEXT_ANYWHERE)
{
	// Yaw180 negates X; the non-zero Origin then moves the point so each axis can be checked independently.
	const FVector Result = FNVectorUtils::TransformPoint(FVector(3.f, 4.f, 0.f), FVector(10.f, 0.f, 2.f), FRotator(0.f, 180.f, 0.f));
	CHECK_MESSAGE(TEXT("TransformPoint Yaw180 should negate X and Y of Point before adding Origin"), Result.Equals(FVector(7.f, -4.f, 2.f), 0.01f));
}

N_TEST_HIGH(FNVectorUtilsTests_RotateAndOffsetPoint_Yaw90WithOffset, "NEXUS::UnitTests::NCore::FNVectorUtils::RotateAndOffsetPoint_Yaw90WithOffset", N_TEST_CONTEXT_ANYWHERE)
{
	// Yaw90 rotates (10,0,0) -> (0,10,0); Offset (5,5,5) shifts to (5,15,5).
	const FVector Result = FNVectorUtils::RotateAndOffsetPoint(FVector(10.f, 0.f, 0.f), FRotator(0.f, 90.f, 0.f), FVector(5.f, 5.f, 5.f));
	CHECK_MESSAGE(TEXT("RotateAndOffsetPoint should rotate Point then add Offset"), Result.Equals(FVector(5.f, 15.f, 5.f), 0.01f));
}

N_TEST_HIGH(FNVectorUtilsTests_RotateAndOffsetPoint_Yaw180WithOffset, "NEXUS::UnitTests::NCore::FNVectorUtils::RotateAndOffsetPoint_Yaw180WithOffset", N_TEST_CONTEXT_ANYWHERE)
{
	// Yaw180 negates X and Y of Point (5,5,0) -> (-5,-5,0); Offset (10,0,0) shifts to (5,-5,0).
	const FVector Result = FNVectorUtils::RotateAndOffsetPoint(FVector(5.f, 5.f, 0.f), FRotator(0.f, 180.f, 0.f), FVector(10.f, 0.f, 0.f));
	CHECK_MESSAGE(TEXT("RotateAndOffsetPoint Yaw180 should negate Point's X and Y before adding Offset"), Result.Equals(FVector(5.f, -5.f, 0.f), 0.01f));
}

N_TEST_HIGH(FNVectorUtilsTests_RotatedAroundPivot_Yaw90AtNonOriginPivot, "NEXUS::UnitTests::NCore::FNVectorUtils::RotatedAroundPivot_Yaw90AtNonOriginPivot", N_TEST_CONTEXT_ANYWHERE)
{
	// World point (10,0,0) relative to pivot (5,5,0) is (5,-5,0); Yaw90 maps that to (5,5,0); pivot adds back to (10,10,0).
	const FVector Result = FNVectorUtils::RotatedAroundPivot(FVector(10.f, 0.f, 0.f), FVector(5.f, 5.f, 0.f), FRotator(0.f, 90.f, 0.f));
	CHECK_MESSAGE(TEXT("RotatedAroundPivot Yaw90 about (5,5,0) should map (10,0,0) to (10,10,0)"), Result.Equals(FVector(10.f, 10.f, 0.f), 0.01f));
}

N_TEST_HIGH(FNVectorUtilsTests_RotatedAroundPivot_Yaw180AtNonOriginPivot, "NEXUS::UnitTests::NCore::FNVectorUtils::RotatedAroundPivot_Yaw180AtNonOriginPivot", N_TEST_CONTEXT_ANYWHERE)
{
	// World point (10,0,0) about pivot (5,0,0): offset (5,0,0); Yaw180 -> (-5,0,0); + pivot = (0,0,0).
	const FVector Result = FNVectorUtils::RotatedAroundPivot(FVector(10.f, 0.f, 0.f), FVector(5.f, 0.f, 0.f), FRotator(0.f, 180.f, 0.f));
	CHECK_MESSAGE(TEXT("RotatedAroundPivot Yaw180 about (5,0,0) should reflect (10,0,0) to (0,0,0)"), Result.Equals(FVector::ZeroVector, 0.01f));
}

N_TEST_HIGH(FNVectorUtilsTests_RotatedAroundPivot_Yaw180AtOrigin, "NEXUS::UnitTests::NCore::FNVectorUtils::RotatedAroundPivot_Yaw180AtOrigin", N_TEST_CONTEXT_ANYWHERE)
{
	// Sanity: Yaw180 at the origin should still negate X.
	const FVector Result = FNVectorUtils::RotatedAroundPivot(FVector(10.f, 0.f, 0.f), FVector::ZeroVector, FRotator(0.f, 180.f, 0.f));
	CHECK_MESSAGE(TEXT("RotatedAroundPivot Yaw180 at origin should negate X"), Result.Equals(FVector(-10.f, 0.f, 0.f), 0.01f));
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
