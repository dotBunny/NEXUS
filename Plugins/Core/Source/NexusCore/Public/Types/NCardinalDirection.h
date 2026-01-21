// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

// #SONARQUBE-DISABLE This file is ignored from duplication checks as this is meant as fast logic

namespace NEXUS::Core::CardinalDirection
{
	constexpr float North = 0.0f;
	constexpr float NorthNorthEast = 22.5f;
	constexpr float NorthEast = 45.0f;
	constexpr float EastNorthEast = 67.5f;
	constexpr float East = 90.0f;
	constexpr float EastSouthEast = 112.5f;
	constexpr float SouthEast = 135.0f;
	constexpr float SouthSouthEast = 157.5f;
	constexpr float South = 180.0f;
	constexpr float SouthSouthWest = 202.5f;
	constexpr float SouthSouthWestNormalized = -157.5f;
	constexpr float SouthWest = 225.0f;
	constexpr float SouthWestNormalized = -135.0f;
	constexpr float WestSouthWest = 247.5f;
	constexpr float WestSouthWestNormalized = -112.5f;
	constexpr float West = 270.0f;
	constexpr float WestNormalized = -90.0f;
	constexpr float WestNorthWest = 292.5f;
	constexpr float WestNorthWestNormalized = -67.5f;
	constexpr float NorthWest = 315.0f;
	constexpr float NorthWestNormalized = -45.0f;
	constexpr float NorthNorthWest = 337.5f;
	constexpr float NorthNorthWestNormalized = -22.5f;
}

UENUM()
enum class ENCardinalDirection : uint8
{
	North = 0,
	NorthNorthEast = 1,
	NorthEast = 2,
	EastNorthEast = 3,
	East = 4,
	EastSouthEast = 5,
	SouthEast = 6,
	SouthSouthEast = 7,
	South = 8,
	SouthSouthWest = 9,
	SouthWest = 10,
	WestSouthWest = 11,
	West = 12,
	WestNorthWest = 13,
	NorthWest = 14,
	NorthNorthWest = 15,
};

class FNCardinalDirectionUtils
{
public:
	static bool IsCardinalAngle(const float Angle)
	{
		return FMath::Modulo(Angle, 22.5f) == 0.0f;
	}
	static double GetClosestCardinalAngle(const float Angle)
	{
		return FMath::RoundToInt(Angle / 22.5f) * 22.5f;
	}
	static FRotator GetClosestCardinalRotator(const FRotator& Rotator)
	{
		return FRotator(
			GetClosestCardinalAngle(Rotator.Pitch),
			GetClosestCardinalAngle(Rotator.Yaw),
			GetClosestCardinalAngle(Rotator.Roll));
	}

	static ENCardinalDirection ToCardinalDirection(const float Angle)
	{
		using enum ENCardinalDirection;
		if (FMath::IsNearlyEqual(Angle, NEXUS::Core::CardinalDirection::North))
		{
			return North;
		}
		if (FMath::IsNearlyEqual(Angle, NEXUS::Core::CardinalDirection::NorthNorthEast))
		{
			return NorthNorthEast;
		}
		if (FMath::IsNearlyEqual(Angle, NEXUS::Core::CardinalDirection::NorthEast))
		{
			return NorthEast;
		}
		if (FMath::IsNearlyEqual(Angle, NEXUS::Core::CardinalDirection::EastNorthEast))
		{
			return EastNorthEast;
		}
		if (FMath::IsNearlyEqual(Angle, NEXUS::Core::CardinalDirection::East))
		{
			return East;
		}
		if (FMath::IsNearlyEqual(Angle, NEXUS::Core::CardinalDirection::EastSouthEast))
		{
			return EastSouthEast;
		}
		if (FMath::IsNearlyEqual(Angle, NEXUS::Core::CardinalDirection::SouthEast))
		{
			return SouthEast;
		}
		if (FMath::IsNearlyEqual(Angle, NEXUS::Core::CardinalDirection::SouthSouthEast))
		{
			return SouthSouthEast;
		}
		if (FMath::IsNearlyEqual(Angle, NEXUS::Core::CardinalDirection::South))
		{
			return South;
		}
		if (FMath::IsNearlyEqual(Angle, NEXUS::Core::CardinalDirection::SouthSouthWest))
		{
			return SouthSouthWest;
		}
		if (FMath::IsNearlyEqual(Angle, NEXUS::Core::CardinalDirection::SouthWest))
		{
			return SouthWest;
		}
		if (FMath::IsNearlyEqual(Angle, NEXUS::Core::CardinalDirection::WestSouthWest))
		{
			return WestSouthWest;
		}
		if (FMath::IsNearlyEqual(Angle, NEXUS::Core::CardinalDirection::West))
		{
			return West;
		}
		if (FMath::IsNearlyEqual(Angle, NEXUS::Core::CardinalDirection::WestNorthWest))
		{
			return WestNorthWest;
		}
		if (FMath::IsNearlyEqual(Angle, NEXUS::Core::CardinalDirection::NorthWest))
		{
			return NorthWest;
		}
		if (FMath::IsNearlyEqual(Angle, NEXUS::Core::CardinalDirection::NorthNorthWest))
		{
			return NorthNorthWest;
		}
		return North;
	}

	static ENCardinalDirection ToCardinalDirectionNormalized(const float NormalizedAngle)
	{
		using enum ENCardinalDirection;
		if (FMath::IsNearlyEqual(NormalizedAngle, NEXUS::Core::CardinalDirection::North))
		{
			return North;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, NEXUS::Core::CardinalDirection::NorthNorthEast))
		{
			return NorthNorthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, NEXUS::Core::CardinalDirection::NorthEast))
		{
			return NorthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, NEXUS::Core::CardinalDirection::EastNorthEast))
		{
			return EastNorthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, NEXUS::Core::CardinalDirection::East))
		{
			return East;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, NEXUS::Core::CardinalDirection::EastSouthEast))
		{
			return EastSouthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, NEXUS::Core::CardinalDirection::SouthEast))
		{
			return SouthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, NEXUS::Core::CardinalDirection::SouthSouthEast))
		{
			return SouthSouthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, NEXUS::Core::CardinalDirection::South) || FMath::IsNearlyEqual(NormalizedAngle, -NEXUS::Core::CardinalDirection::South))
		{
			return South;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, NEXUS::Core::CardinalDirection::SouthSouthWestNormalized))
		{
			return SouthSouthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, NEXUS::Core::CardinalDirection::SouthWestNormalized))
		{
			return SouthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, NEXUS::Core::CardinalDirection::WestSouthWestNormalized))
		{
			return WestSouthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, NEXUS::Core::CardinalDirection::WestNormalized))
		{
			return West;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, NEXUS::Core::CardinalDirection::WestNorthWestNormalized))
		{
			return WestNorthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, NEXUS::Core::CardinalDirection::NorthWestNormalized))
		{
			return NorthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, NEXUS::Core::CardinalDirection::NorthNorthWestNormalized))
		{
			return NorthNorthWest;
		}
		return North;

	}
	
	static float ToDecimalDegrees(const ENCardinalDirection CardinalDirection)
	{
		switch (CardinalDirection)
		{
			using enum ENCardinalDirection;
		case North:
			return NEXUS::Core::CardinalDirection::North;
		case NorthNorthEast:
			return NEXUS::Core::CardinalDirection::NorthNorthEast;
		case NorthEast:
			return NEXUS::Core::CardinalDirection::NorthEast;
		case EastNorthEast:
			return NEXUS::Core::CardinalDirection::EastNorthEast;
		case East:
			return NEXUS::Core::CardinalDirection::East;
		case EastSouthEast:
			return NEXUS::Core::CardinalDirection::EastSouthEast;
		case SouthEast:
			return NEXUS::Core::CardinalDirection::SouthEast;
		case SouthSouthEast:
			return NEXUS::Core::CardinalDirection::SouthSouthEast;
		case South:
			return NEXUS::Core::CardinalDirection::South;
		case SouthSouthWest:
			return NEXUS::Core::CardinalDirection::SouthSouthWest;
		case SouthWest:
			return NEXUS::Core::CardinalDirection::SouthWest;
		case WestSouthWest:
			return NEXUS::Core::CardinalDirection::WestSouthWest;
		case West:
			return NEXUS::Core::CardinalDirection::West;
		case WestNorthWest:
			return NEXUS::Core::CardinalDirection::WestNorthWest;
		case NorthWest:
			return NEXUS::Core::CardinalDirection::NorthWest;
		case NorthNorthWest:
			return NEXUS::Core::CardinalDirection::NorthNorthWest;
		}
		return NEXUS::Core::CardinalDirection::North;
	}

	static float ToDecimalDegreesNormalized(const ENCardinalDirection CardinalDirection)
	{
		switch (CardinalDirection)
		{
			using enum ENCardinalDirection;
		case SouthSouthWest:
			return NEXUS::Core::CardinalDirection::SouthSouthWestNormalized;
		case SouthWest:
			return NEXUS::Core::CardinalDirection::SouthWestNormalized;
		case WestSouthWest:
			return NEXUS::Core::CardinalDirection::WestSouthWestNormalized;
		case West:
			return NEXUS::Core::CardinalDirection::WestNormalized;
		case WestNorthWest:
			return NEXUS::Core::CardinalDirection::WestNorthWestNormalized;
		case NorthWest:
			return NEXUS::Core::CardinalDirection::NorthWestNormalized;
		case NorthNorthWest:
			return NEXUS::Core::CardinalDirection::NorthNorthWestNormalized;
		case North:
			return NEXUS::Core::CardinalDirection::North;
		case NorthNorthEast:
			return NEXUS::Core::CardinalDirection::NorthNorthEast;
		case NorthEast:
			return NEXUS::Core::CardinalDirection::NorthEast;
		case EastNorthEast:
			return NEXUS::Core::CardinalDirection::EastNorthEast;
		case East:
			return NEXUS::Core::CardinalDirection::East;
		case EastSouthEast:
			return NEXUS::Core::CardinalDirection::EastSouthEast;
		case SouthEast:
			return NEXUS::Core::CardinalDirection::SouthEast;
		case SouthSouthEast:
			return NEXUS::Core::CardinalDirection::SouthSouthEast;
		case South:
			return NEXUS::Core::CardinalDirection::South;

		}
		return NEXUS::Core::CardinalDirection::North;
	}
};

// #SONARQUBE-ENABLE