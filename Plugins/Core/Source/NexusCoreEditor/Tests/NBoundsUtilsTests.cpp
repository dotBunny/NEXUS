// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Math/NBoundsUtils.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(FNBoundsUtilsTests_IsBoundsContainedInBounds_FullyInside, "NEXUS::UnitTests::NCore::FNBoundsUtils::IsBoundsContainedInBounds_FullyInside", N_TEST_CONTEXT_ANYWHERE)
{
	const FBoxSphereBounds Inner(FVector::ZeroVector, FVector(10.f), 10.f);
	const FBoxSphereBounds Outer(FVector::ZeroVector, FVector(100.f), 100.f);
	CHECK_MESSAGE(TEXT("Smaller bounds centered at origin should be contained in larger bounds"), FNBoundsUtils::IsBoundsContainedInBounds(Inner, Outer));
}

N_TEST_HIGH(FNBoundsUtilsTests_IsBoundsContainedInBounds_OutsideBounds, "NEXUS::UnitTests::NCore::FNBoundsUtils::IsBoundsContainedInBounds_OutsideBounds", N_TEST_CONTEXT_ANYWHERE)
{
	const FBoxSphereBounds Inner(FVector(500.f, 0.f, 0.f), FVector(10.f), 10.f);
	const FBoxSphereBounds Outer(FVector::ZeroVector, FVector(100.f), 100.f);
	CHECK_FALSE_MESSAGE(TEXT("Offset bounds should not be contained"), FNBoundsUtils::IsBoundsContainedInBounds(Inner, Outer));
}

N_TEST_HIGH(FNBoundsUtilsTests_IsBoundsContainedInBounds_LargerInSmaller, "NEXUS::UnitTests::NCore::FNBoundsUtils::IsBoundsContainedInBounds_LargerInSmaller", N_TEST_CONTEXT_ANYWHERE)
{
	const FBoxSphereBounds Inner(FVector::ZeroVector, FVector(100.f), 100.f);
	const FBoxSphereBounds Outer(FVector::ZeroVector, FVector(10.f), 10.f);
	CHECK_FALSE_MESSAGE(TEXT("Larger bounds should not be contained in smaller bounds"), FNBoundsUtils::IsBoundsContainedInBounds(Inner, Outer));
}

N_TEST_HIGH(FNBoundsUtilsTests_IsBoundsContainedInBounds_Identical, "NEXUS::UnitTests::NCore::FNBoundsUtils::IsBoundsContainedInBounds_Identical", N_TEST_CONTEXT_ANYWHERE)
{
	const FBoxSphereBounds Bounds(FVector::ZeroVector, FVector(50.f), 50.f);
	CHECK_MESSAGE(TEXT("Identical bounds should be contained in themselves"), FNBoundsUtils::IsBoundsContainedInBounds(Bounds, Bounds));
}

N_TEST_HIGH(FNBoundsUtilsTests_IsPointInBounds_Inside, "NEXUS::UnitTests::NCore::FNBoundsUtils::IsPointInBounds_Inside", N_TEST_CONTEXT_ANYWHERE)
{
	const FBoxSphereBounds Bounds(FVector::ZeroVector, FVector(100.f), 100.f);
	CHECK_MESSAGE(TEXT("Origin should be inside bounds centered at origin"), FNBoundsUtils::IsPointInBounds(FVector::ZeroVector, Bounds));
	CHECK_MESSAGE(TEXT("Point near center should be inside"), FNBoundsUtils::IsPointInBounds(FVector(10.f, 10.f, 10.f), Bounds));
}

N_TEST_HIGH(FNBoundsUtilsTests_IsPointInBounds_Outside, "NEXUS::UnitTests::NCore::FNBoundsUtils::IsPointInBounds_Outside", N_TEST_CONTEXT_ANYWHERE)
{
	const FBoxSphereBounds Bounds(FVector::ZeroVector, FVector(10.f), 10.f);
	CHECK_FALSE_MESSAGE(TEXT("Distant point should be outside bounds"), FNBoundsUtils::IsPointInBounds(FVector(500.f, 0.f, 0.f), Bounds));
}

N_TEST_HIGH(FNBoundsUtilsTests_GetPointInBounds_InsideReturnsSame, "NEXUS::UnitTests::NCore::FNBoundsUtils::GetPointInBounds_InsideReturnsSame", N_TEST_CONTEXT_ANYWHERE)
{
	const FBoxSphereBounds Bounds(FVector::ZeroVector, FVector(100.f), 100.f);
	const FVector Point(10.f, 10.f, 10.f);
	const FVector Result = FNBoundsUtils::GetPointInBounds(Point, Bounds);
	CHECK_MESSAGE(TEXT("Point already inside should be returned unchanged"), Point.Equals(Result, 0.01f));
}

N_TEST_HIGH(FNBoundsUtilsTests_GetPointInBounds_OutsideClamps, "NEXUS::UnitTests::NCore::FNBoundsUtils::GetPointInBounds_OutsideClamps", N_TEST_CONTEXT_ANYWHERE)
{
	const FBoxSphereBounds Bounds(FVector::ZeroVector, FVector(10.f), 10.f);
	const FVector Far(500.f, 0.f, 0.f);
	const FVector Result = FNBoundsUtils::GetPointInBounds(Far, Bounds);
	CHECK_MESSAGE(TEXT("Clamped point should be inside bounds"), FNBoundsUtils::IsPointInBounds(Result, Bounds));
}

N_TEST_MEDIUM(FNBoundsUtilsTests_GetPointInBoundsWithMargin_ShrinksRegion, "NEXUS::UnitTests::NCore::FNBoundsUtils::GetPointInBoundsWithMargin_ShrinksRegion", N_TEST_CONTEXT_ANYWHERE)
{
	const FBoxSphereBounds Bounds(FVector::ZeroVector, FVector(100.f), 100.f);
	const FVector Margin(20.f, 20.f, 20.f);
	const FVector EdgePoint(95.f, 0.f, 0.f);
	const FVector Result = FNBoundsUtils::GetPointInBoundsWithMargin(EdgePoint, Bounds, Margin);
	CHECK_MESSAGE(TEXT("Point near edge should be clamped inward by margin"), Result.X <= 80.f + 0.01f);
}

#endif //WITH_TESTS
