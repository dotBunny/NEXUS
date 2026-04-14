// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#if WITH_TESTS

#include "Types/NCardinalDirection.h"
#include "Macros/NTestMacros.h"

N_TEST_HIGH(FNCardinalDirectionTests_IsCardinalAngle_ExactAngles, "NEXUS::UnitTests::NCore::CardinalDirection::IsCardinalAngle_ExactAngles", N_TEST_CONTEXT_ANYWHERE)
{
	// All 22.5-degree increments are cardinal angles
	CHECK_MESSAGE(TEXT("0 degrees should be a cardinal angle"), FNCardinalDirectionUtils::IsCardinalAngle(0.f));
	CHECK_MESSAGE(TEXT("22.5 degrees should be a cardinal angle"), FNCardinalDirectionUtils::IsCardinalAngle(22.5f));
	CHECK_MESSAGE(TEXT("45 degrees should be a cardinal angle"), FNCardinalDirectionUtils::IsCardinalAngle(45.f));
	CHECK_MESSAGE(TEXT("90 degrees should be a cardinal angle"), FNCardinalDirectionUtils::IsCardinalAngle(90.f));
	CHECK_MESSAGE(TEXT("180 degrees should be a cardinal angle"), FNCardinalDirectionUtils::IsCardinalAngle(180.f));
	CHECK_MESSAGE(TEXT("270 degrees should be a cardinal angle"), FNCardinalDirectionUtils::IsCardinalAngle(270.f));
	CHECK_MESSAGE(TEXT("337.5 degrees should be a cardinal angle"), FNCardinalDirectionUtils::IsCardinalAngle(337.5f));
}

N_TEST_HIGH(FNCardinalDirectionTests_IsCardinalAngle_NonCardinalAngles, "NEXUS::UnitTests::NCore::CardinalDirection::IsCardinalAngle_NonCardinalAngles", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_FALSE_MESSAGE(TEXT("10 degrees should not be a cardinal angle"), FNCardinalDirectionUtils::IsCardinalAngle(10.f));
	CHECK_FALSE_MESSAGE(TEXT("15 degrees should not be a cardinal angle"), FNCardinalDirectionUtils::IsCardinalAngle(15.f));
	CHECK_FALSE_MESSAGE(TEXT("100 degrees should not be a cardinal angle"), FNCardinalDirectionUtils::IsCardinalAngle(100.f));
	CHECK_FALSE_MESSAGE(TEXT("123.7 degrees should not be a cardinal angle"), FNCardinalDirectionUtils::IsCardinalAngle(123.7f));
}

N_TEST_HIGH(FNCardinalDirectionTests_GetClosestCardinalAngle_OnCardinal, "NEXUS::UnitTests::NCore::CardinalDirection::GetClosestCardinalAngle_OnCardinal", N_TEST_CONTEXT_ANYWHERE)
{
	// Exact cardinal angles should return themselves
	CHECK_MESSAGE(TEXT("0 degrees should snap to 0"), FMath::IsNearlyEqual(FNCardinalDirectionUtils::GetClosestCardinalAngle(0.f), 0.0, 0.01));
	CHECK_MESSAGE(TEXT("90 degrees should snap to 90"), FMath::IsNearlyEqual(FNCardinalDirectionUtils::GetClosestCardinalAngle(90.f), 90.0, 0.01));
	CHECK_MESSAGE(TEXT("180 degrees should snap to 180"), FMath::IsNearlyEqual(FNCardinalDirectionUtils::GetClosestCardinalAngle(180.f), 180.0, 0.01));
}

N_TEST_HIGH(FNCardinalDirectionTests_GetClosestCardinalAngle_OffCardinal, "NEXUS::UnitTests::NCore::CardinalDirection::GetClosestCardinalAngle_OffCardinal", N_TEST_CONTEXT_ANYWHERE)
{
	// 10 degrees is closer to 0 than to 22.5
	CHECK_MESSAGE(TEXT("10 degrees should snap to nearest cardinal (0)"), FMath::IsNearlyEqual(FNCardinalDirectionUtils::GetClosestCardinalAngle(10.f), 0.0, 0.01));
	// 80 degrees is closer to 90 than to 67.5
	CHECK_MESSAGE(TEXT("80 degrees should snap to nearest cardinal (90)"), FMath::IsNearlyEqual(FNCardinalDirectionUtils::GetClosestCardinalAngle(80.f), 90.0, 0.01));
	// 34 degrees is closer to 45 than to 22.5
	CHECK_MESSAGE(TEXT("34 degrees should snap to nearest cardinal (45)"), FMath::IsNearlyEqual(FNCardinalDirectionUtils::GetClosestCardinalAngle(34.f), 45.0, 0.01));
}

N_TEST_HIGH(FNCardinalDirectionTests_GetClosestCardinalRotator, "NEXUS::UnitTests::NCore::CardinalDirection::GetClosestCardinalRotator", N_TEST_CONTEXT_ANYWHERE)
{
	const FRotator Input(10.f, 80.f, 34.f);
	const FRotator Result = FNCardinalDirectionUtils::GetClosestCardinalRotator(Input);
	CHECK_MESSAGE(TEXT("Rotator pitch should snap to nearest cardinal"), FMath::IsNearlyEqual(Result.Pitch, 0.0, 0.01));
	CHECK_MESSAGE(TEXT("Rotator yaw should snap to nearest cardinal"), FMath::IsNearlyEqual(Result.Yaw, 90.0, 0.01));
	CHECK_MESSAGE(TEXT("Rotator roll should snap to nearest cardinal"), FMath::IsNearlyEqual(Result.Roll, 45.0, 0.01));
}

N_TEST_HIGH(FNCardinalDirectionTests_ToCardinalDirection_AllDirections, "NEXUS::UnitTests::NCore::CardinalDirection::ToCardinalDirection_AllDirections", N_TEST_CONTEXT_ANYWHERE)
{
	CHECK_MESSAGE(TEXT("0 should map to North"), FNCardinalDirectionUtils::ToCardinalDirection(0.f) == ENCardinalDirection::North);
	CHECK_MESSAGE(TEXT("22.5 should map to NorthNorthEast"), FNCardinalDirectionUtils::ToCardinalDirection(22.5f) == ENCardinalDirection::NorthNorthEast);
	CHECK_MESSAGE(TEXT("45 should map to NorthEast"), FNCardinalDirectionUtils::ToCardinalDirection(45.f) == ENCardinalDirection::NorthEast);
	CHECK_MESSAGE(TEXT("67.5 should map to EastNorthEast"), FNCardinalDirectionUtils::ToCardinalDirection(67.5f) == ENCardinalDirection::EastNorthEast);
	CHECK_MESSAGE(TEXT("90 should map to East"), FNCardinalDirectionUtils::ToCardinalDirection(90.f) == ENCardinalDirection::East);
	CHECK_MESSAGE(TEXT("135 should map to SouthEast"), FNCardinalDirectionUtils::ToCardinalDirection(135.f) == ENCardinalDirection::SouthEast);
	CHECK_MESSAGE(TEXT("180 should map to South"), FNCardinalDirectionUtils::ToCardinalDirection(180.f) == ENCardinalDirection::South);
	CHECK_MESSAGE(TEXT("225 should map to SouthWest"), FNCardinalDirectionUtils::ToCardinalDirection(225.f) == ENCardinalDirection::SouthWest);
	CHECK_MESSAGE(TEXT("270 should map to West"), FNCardinalDirectionUtils::ToCardinalDirection(270.f) == ENCardinalDirection::West);
	CHECK_MESSAGE(TEXT("315 should map to NorthWest"), FNCardinalDirectionUtils::ToCardinalDirection(315.f) == ENCardinalDirection::NorthWest);
	CHECK_MESSAGE(TEXT("337.5 should map to NorthNorthWest"), FNCardinalDirectionUtils::ToCardinalDirection(337.5f) == ENCardinalDirection::NorthNorthWest);
}

N_TEST_HIGH(FNCardinalDirectionTests_ToDecimalDegrees_RoundTrip, "NEXUS::UnitTests::NCore::CardinalDirection::ToDecimalDegrees_RoundTrip", N_TEST_CONTEXT_ANYWHERE)
{
	// ToCardinalDirection and ToDecimalDegrees should form a round-trip for exact angles
	constexpr float Epsilon = 0.01f;
	const TArray<float> Angles = {0.f, 22.5f, 45.f, 67.5f, 90.f, 112.5f, 135.f, 157.5f, 180.f, 202.5f, 225.f, 247.5f, 270.f, 292.5f, 315.f, 337.5f};
	for (const float Angle : Angles)
	{
		const ENCardinalDirection Dir = FNCardinalDirectionUtils::ToCardinalDirection(Angle);
		const float BackToAngle = FNCardinalDirectionUtils::ToDecimalDegrees(Dir);
		CHECK_MESSAGE(FString::Printf(TEXT("Round-trip for angle %f should be lossless"), Angle), FMath::IsNearlyEqual(BackToAngle, Angle, Epsilon));
	}
}

N_TEST_HIGH(FNCardinalDirectionTests_ToCardinalDirectionNormalized_NegativeAngles, "NEXUS::UnitTests::NCore::CardinalDirection::ToCardinalDirectionNormalized_NegativeAngles", N_TEST_CONTEXT_ANYWHERE)
{
	// Normalized range uses negative values for angles > 180
	CHECK_MESSAGE(TEXT("-90 should map to West"), FNCardinalDirectionUtils::ToCardinalDirectionNormalized(-90.f) == ENCardinalDirection::West);
	CHECK_MESSAGE(TEXT("-135 should map to SouthWest"), FNCardinalDirectionUtils::ToCardinalDirectionNormalized(-135.f) == ENCardinalDirection::SouthWest);
	CHECK_MESSAGE(TEXT("-45 should map to NorthWest"), FNCardinalDirectionUtils::ToCardinalDirectionNormalized(-45.f) == ENCardinalDirection::NorthWest);
	CHECK_MESSAGE(TEXT("-22.5 should map to NorthNorthWest"), FNCardinalDirectionUtils::ToCardinalDirectionNormalized(-22.5f) == ENCardinalDirection::NorthNorthWest);
}

N_TEST_HIGH(FNCardinalDirectionTests_ToDecimalDegreesNormalized_WesternDirections, "NEXUS::UnitTests::NCore::CardinalDirection::ToDecimalDegreesNormalized_WesternDirections", N_TEST_CONTEXT_ANYWHERE)
{
	// Western directions return negative values in normalized form
	CHECK_MESSAGE(TEXT("West should normalize to -90"), FMath::IsNearlyEqual(FNCardinalDirectionUtils::ToDecimalDegreesNormalized(ENCardinalDirection::West), -90.f, 0.01f));
	CHECK_MESSAGE(TEXT("SouthWest should normalize to -135"), FMath::IsNearlyEqual(FNCardinalDirectionUtils::ToDecimalDegreesNormalized(ENCardinalDirection::SouthWest), -135.f, 0.01f));
	CHECK_MESSAGE(TEXT("NorthWest should normalize to -45"), FMath::IsNearlyEqual(FNCardinalDirectionUtils::ToDecimalDegreesNormalized(ENCardinalDirection::NorthWest), -45.f, 0.01f));
}

N_TEST_HIGH(FNCardinalDirectionTests_ToDecimalDegreesNormalized_EasternDirections, "NEXUS::UnitTests::NCore::CardinalDirection::ToDecimalDegreesNormalized_EasternDirections", N_TEST_CONTEXT_ANYWHERE)
{
	// Eastern directions remain positive in normalized form
	CHECK_MESSAGE(TEXT("North should normalize to 0"), FMath::IsNearlyEqual(FNCardinalDirectionUtils::ToDecimalDegreesNormalized(ENCardinalDirection::North), 0.f, 0.01f));
	CHECK_MESSAGE(TEXT("East should normalize to 90"), FMath::IsNearlyEqual(FNCardinalDirectionUtils::ToDecimalDegreesNormalized(ENCardinalDirection::East), 90.f, 0.01f));
	CHECK_MESSAGE(TEXT("South should normalize to 180"), FMath::IsNearlyEqual(FNCardinalDirectionUtils::ToDecimalDegreesNormalized(ENCardinalDirection::South), 180.f, 0.01f));
}

#endif //WITH_TESTS
