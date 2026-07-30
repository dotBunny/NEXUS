// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NVectorUtils.h"
#include "Macros/NTestMacros.h"
#include "Tests/TestHarnessAdapter.h"

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

N_TEST_HIGH(FNVectorUtilsTests_RotateAndOffsetPoints_Yaw90WithOffset,
	"NEXUS::UnitTests::NCore::FNVectorUtils::RotateAndOffsetPoints::Yaw90WithOffset",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Batch variant of RotateAndOffsetPoint — every input must come back rotated-then-offset, in order.
	const TArray<FVector> Inputs = { FVector(10, 0, 0), FVector(0, 10, 0), FVector(5, 5, 0) };
	const TArray<FVector> Result = FNVectorUtils::RotateAndOffsetPoints(Inputs, FRotator(0.f, 90.f, 0.f), FVector(1, 1, 1));

	CHECK_EQUALS("Output array length must match the input length", Result.Num(), 3);
	CHECK_MESSAGE(TEXT("Index 0: yaw90 maps (10,0,0)→(0,10,0); +offset → (1,11,1)"), Result[0].Equals(FVector(1, 11, 1), 0.01));
	CHECK_MESSAGE(TEXT("Index 1: yaw90 maps (0,10,0)→(-10,0,0); +offset → (-9,1,1)"), Result[1].Equals(FVector(-9, 1, 1), 0.01));
	CHECK_MESSAGE(TEXT("Index 2: yaw90 maps (5,5,0)→(-5,5,0); +offset → (-4,6,1)"), Result[2].Equals(FVector(-4, 6, 1), 0.01));
}

N_TEST_HIGH(FNVectorUtilsTests_RotateAndOffsetPoints_Empty_ReturnsEmpty,
	"NEXUS::UnitTests::NCore::FNVectorUtils::RotateAndOffsetPoints::Empty_ReturnsEmpty",
	N_TEST_CONTEXT_ANYWHERE)
{
	const TArray<FVector> Empty;
	const TArray<FVector> Result = FNVectorUtils::RotateAndOffsetPoints(Empty, FRotator(0.f, 90.f, 0.f), FVector(1, 1, 1));
	CHECK_EQUALS("Empty input must yield an empty output without crashing", Result.Num(), 0);
}

N_TEST_HIGH(FNVectorUtilsTests_RotatePoints_Yaw180,
	"NEXUS::UnitTests::NCore::FNVectorUtils::RotatePoints::Yaw180",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Yaw180 negates X and Y component-wise; Z is untouched. Batch variant must preserve order.
	const TArray<FVector> Inputs = { FVector(3, 4, 5), FVector(-1, 2, 0) };
	const TArray<FVector> Result = FNVectorUtils::RotatePoints(Inputs, FRotator(0.f, 180.f, 0.f));

	CHECK_EQUALS("Output length must match input length", Result.Num(), 2);
	CHECK_MESSAGE(TEXT("Yaw180 must negate X and Y of vertex 0 while leaving Z intact"), Result[0].Equals(FVector(-3, -4, 5), 0.01));
	CHECK_MESSAGE(TEXT("Yaw180 must negate X and Y of vertex 1 while leaving Z intact"), Result[1].Equals(FVector(1, -2, 0), 0.01));
}

N_TEST_HIGH(FNVectorUtilsTests_OffsetPoints_AppliesPerVector,
	"NEXUS::UnitTests::NCore::FNVectorUtils::OffsetPoints::AppliesPerVector",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Pure translation — the offset must apply to every entry independently with no implicit rotation.
	const TArray<FVector> Inputs = { FVector(0, 0, 0), FVector(10, 0, 0), FVector(0, 10, 0) };
	const TArray<FVector> Result = FNVectorUtils::OffsetPoints(Inputs, FVector(5, -5, 1));

	CHECK_EQUALS("Output length must match input length", Result.Num(), 3);
	CHECK_MESSAGE(TEXT("Vertex 0 must equal Offset"), Result[0].Equals(FVector(5, -5, 1)));
	CHECK_MESSAGE(TEXT("Vertex 1 must equal Inputs[1] + Offset"), Result[1].Equals(FVector(15, -5, 1)));
	CHECK_MESSAGE(TEXT("Vertex 2 must equal Inputs[2] + Offset"), Result[2].Equals(FVector(5, 5, 1)));
}

N_TEST_HIGH(FNVectorUtilsTests_OffsetPoints_ZeroOffset_NoOp,
	"NEXUS::UnitTests::NCore::FNVectorUtils::OffsetPoints::ZeroOffset_NoOp",
	N_TEST_CONTEXT_ANYWHERE)
{
	const TArray<FVector> Inputs = { FVector(1, 2, 3), FVector(4, 5, 6) };
	const TArray<FVector> Result = FNVectorUtils::OffsetPoints(Inputs, FVector::ZeroVector);

	CHECK_MESSAGE(TEXT("Zero offset must leave vertex 0 unchanged"), Result[0].Equals(Inputs[0]));
	CHECK_MESSAGE(TEXT("Zero offset must leave vertex 1 unchanged"), Result[1].Equals(Inputs[1]));
}

N_TEST_HIGH(FNVectorUtilsTests_GetFurthestGridIntersection_OffGrid_ReturnsCeiledUnitIndices,
	"NEXUS::UnitTests::NCore::FNVectorUtils::GetFurthestGridIntersection::OffGrid_ReturnsCeiledUnitIndices",
	N_TEST_CONTEXT_ANYWHERE)
{
	// IMPORTANT: despite the docstring saying "grid intersection", this returns the per-axis unit INDEX
	// (the GetCrunchedGridUnit result), not a world-space coordinate. WorldAssembly's UnitBounds
	// construction (NWorldAssemblyUtils.cpp:165, NCellActor.cpp:102) depends on this index behaviour.
	// For (74,74,74) on a 50-unit grid: 74/50 = 1.48 → ceil → 2 per axis.
	const FVector GridSize(50.f, 50.f, 50.f);
	const FVector Result = FNVectorUtils::GetFurthestGridIntersection(FVector(74, 74, 74), GridSize);
	CHECK_MESSAGE(TEXT("(74,74,74) on a 50-grid must crunch to unit indices (2,2,2)"), Result.Equals(FVector(2, 2, 2), 0.001));
}

N_TEST_HIGH(FNVectorUtilsTests_GetFurthestGridIntersection_OnGrid_ReturnsExactUnitIndices,
	"NEXUS::UnitTests::NCore::FNVectorUtils::GetFurthestGridIntersection::OnGrid_ReturnsExactUnitIndices",
	N_TEST_CONTEXT_ANYWHERE)
{
	// Exactly-on-grid input hits GetCrunchedGridUnit's IsNearlyEqual branch and returns the exact unit
	// index (no ceil). For (100,150,200) on a 50-grid: (2,3,4).
	const FVector GridSize(50.f, 50.f, 50.f);
	const FVector Result = FNVectorUtils::GetFurthestGridIntersection(FVector(100, 150, 200), GridSize);
	CHECK_MESSAGE(TEXT("On-grid input must return exact unit indices (2,3,4) — IsNearlyEqual branch"),
		Result.Equals(FVector(2, 3, 4), 0.001));
}

N_TEST_HIGH(FNVectorUtilsTests_GetFurthestGridIntersection_Zero_ReturnsZero,
	"NEXUS::UnitTests::NCore::FNVectorUtils::GetFurthestGridIntersection::Zero_ReturnsZero",
	N_TEST_CONTEXT_ANYWHERE)
{
	const FVector GridSize(50.f, 50.f, 50.f);
	const FVector Result = FNVectorUtils::GetFurthestGridIntersection(FVector::ZeroVector, GridSize);
	CHECK_MESSAGE(TEXT("Zero input must return zero unit indices (the IsNearlyZero short-circuit)"),
		Result.Equals(FVector::ZeroVector, 0.001));
}

N_TEST_HIGH(FNVectorUtilsTests_GetCrunchedGridUnit_Negative_CeilsTowardZero,
	"NEXUS::UnitTests::NCore::FNVectorUtils::GetCrunchedGridUnit::Negative_CeilsTowardZero",
	N_TEST_CONTEXT_ANYWHERE)
{
	// CeilToInt(-1.5) = -1 (rounds toward zero for negatives) — locks that the "crunch" direction for
	// negative values is toward zero, not away from it. A caller using this to walk grid cells in -X needs
	// this contract to be stable.
	CHECK_EQUALS("CeilToInt of -1.5 should round toward zero (-1)", FNVectorUtils::GetCrunchedGridUnit(-75.0, 50.0), -1);
}

N_TEST_HIGH(FNVectorUtilsTests_GetCrunchedGridUnit_NegativeOnGrid_ReturnsExactUnit,
	"NEXUS::UnitTests::NCore::FNVectorUtils::GetCrunchedGridUnit::NegativeOnGrid_ReturnsExactUnit",
	N_TEST_CONTEXT_ANYWHERE)
{
	// -100 / 50 = -2 exactly — IsNearlyEqual branch must return the exact value, not ceil.
	CHECK_EQUALS("Negative on-grid value should return its exact grid unit", FNVectorUtils::GetCrunchedGridUnit(-100.0, 50.0), -2);
}

#endif //WITH_TESTS
