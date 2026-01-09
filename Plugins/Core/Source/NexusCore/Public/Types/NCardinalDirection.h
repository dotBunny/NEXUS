// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

namespace Nexus::Core::CardinalDirection
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
	constexpr float SouthWest = 225.0f;
	constexpr float WestSouthWest = 247.5f;
	constexpr float West = 270.0f;
	constexpr float WestNorthWest = 292.5f;
	constexpr float NorthWest = 315.0f;
	constexpr float NorthNorthWest = 337.5f;
}

namespace Nexus::Core::CardinalDirectionNormalized
{
	constexpr float SouthSouthWest = -157.5f;
	constexpr float SouthWest = -135.0f;
	constexpr float WestSouthWest = -112.5f;
	constexpr float West = -90.0f;
	constexpr float WestNorthWest = -67.5f;
	constexpr float NorthWest = -45.0f;
	constexpr float NorthNorthWest = -22.5f;
}

UENUM()
enum class ENCardinalDirection : uint8
{
	CD_North = 0				UMETA(DisplayName = "North"),
	CD_NorthNorthEast = 1		UMETA(DisplayName = "North North East"),
	CD_NorthEast = 2			UMETA(DisplayName = "North East"),
	CD_EastNorthEast = 3		UMETA(DisplayName = "East North East"),
	CD_East	= 4					UMETA(DisplayName = "East"),
	CD_EastSouthEast = 5		UMETA(DisplayName = "East South East"),
	CD_SouthEast = 6			UMETA(DisplayName = "South East"),
	CD_SouthSouthEast = 7		UMETA(DisplayName = "South South East"),
	CD_South = 8				UMETA(DisplayName = "South"),
	CD_SouthSouthWest = 9		UMETA(DisplayName = "South South West"),
	CD_SouthWest = 10			UMETA(DisplayName = "South West"),
	CD_WestSouthWest = 11		UMETA(DisplayName = "West South West"),
	CD_West	= 12				UMETA(DisplayName = "West"),
	CD_WestNorthWest = 13		UMETA(DisplayName = "West North West"),
	CD_NorthWest = 14			UMETA(DisplayName = "North West"),
	CD_NorthNorthWest = 15		UMETA(DisplayName = "North North West"),
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
		if (FMath::IsNearlyEqual(Angle, Nexus::Core::CardinalDirection::North))
		{
			return ENCardinalDirection::CD_North;
		}
		if (FMath::IsNearlyEqual(Angle, Nexus::Core::CardinalDirection::NorthNorthEast))
		{
			return ENCardinalDirection::CD_NorthNorthEast;
		}
		if (FMath::IsNearlyEqual(Angle, Nexus::Core::CardinalDirection::NorthEast))
		{
			return ENCardinalDirection::CD_NorthEast;
		}
		if (FMath::IsNearlyEqual(Angle, Nexus::Core::CardinalDirection::EastNorthEast))
		{
			return ENCardinalDirection::CD_EastNorthEast;
		}
		if (FMath::IsNearlyEqual(Angle, Nexus::Core::CardinalDirection::East))
		{
			return ENCardinalDirection::CD_East;
		}
		if (FMath::IsNearlyEqual(Angle, Nexus::Core::CardinalDirection::EastSouthEast))
		{
			return ENCardinalDirection::CD_EastSouthEast;
		}
		if (FMath::IsNearlyEqual(Angle, Nexus::Core::CardinalDirection::SouthEast))
		{
			return ENCardinalDirection::CD_SouthEast;
		}
		if (FMath::IsNearlyEqual(Angle, Nexus::Core::CardinalDirection::SouthSouthEast))
		{
			return ENCardinalDirection::CD_SouthSouthEast;
		}
		if (FMath::IsNearlyEqual(Angle, Nexus::Core::CardinalDirection::South))
		{
			return ENCardinalDirection::CD_South;
		}
		if (FMath::IsNearlyEqual(Angle, Nexus::Core::CardinalDirection::SouthSouthWest))
		{
			return ENCardinalDirection::CD_SouthSouthWest;
		}
		if (FMath::IsNearlyEqual(Angle, Nexus::Core::CardinalDirection::SouthWest))
		{
			return ENCardinalDirection::CD_SouthWest;
		}
		if (FMath::IsNearlyEqual(Angle, Nexus::Core::CardinalDirection::WestSouthWest))
		{
			return ENCardinalDirection::CD_WestSouthWest;
		}
		if (FMath::IsNearlyEqual(Angle, Nexus::Core::CardinalDirection::West))
		{
			return ENCardinalDirection::CD_West;
		}
		if (FMath::IsNearlyEqual(Angle, Nexus::Core::CardinalDirection::WestNorthWest))
		{
			return ENCardinalDirection::CD_WestNorthWest;
		}
		if (FMath::IsNearlyEqual(Angle, Nexus::Core::CardinalDirection::NorthWest))
		{
			return ENCardinalDirection::CD_NorthWest;
		}
		if (FMath::IsNearlyEqual(Angle, Nexus::Core::CardinalDirection::NorthNorthWest))
		{
			return ENCardinalDirection::CD_NorthNorthWest;
		}
		return ENCardinalDirection::CD_North;
	}

	static ENCardinalDirection ToCardinalDirectionNormalized(const float NormalizedAngle)
	{
		if (FMath::IsNearlyEqual(NormalizedAngle, Nexus::Core::CardinalDirection::North))
		{
			return ENCardinalDirection::CD_North;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, Nexus::Core::CardinalDirection::NorthNorthEast))
		{
			return ENCardinalDirection::CD_NorthNorthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, Nexus::Core::CardinalDirection::NorthEast))
		{
			return ENCardinalDirection::CD_NorthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, Nexus::Core::CardinalDirection::EastNorthEast))
		{
			return ENCardinalDirection::CD_EastNorthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, Nexus::Core::CardinalDirection::East))
		{
			return ENCardinalDirection::CD_East;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, Nexus::Core::CardinalDirection::EastSouthEast))
		{
			return ENCardinalDirection::CD_EastSouthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, Nexus::Core::CardinalDirection::SouthEast))
		{
			return ENCardinalDirection::CD_SouthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, Nexus::Core::CardinalDirection::SouthSouthEast))
		{
			return ENCardinalDirection::CD_SouthSouthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, Nexus::Core::CardinalDirection::South) || FMath::IsNearlyEqual(NormalizedAngle, -Nexus::Core::CardinalDirection::South))
		{
			return ENCardinalDirection::CD_South;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, Nexus::Core::CardinalDirectionNormalized::SouthSouthWest))
		{
			return ENCardinalDirection::CD_SouthSouthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, Nexus::Core::CardinalDirectionNormalized::SouthWest))
		{
			return ENCardinalDirection::CD_SouthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, Nexus::Core::CardinalDirectionNormalized::WestSouthWest))
		{
			return ENCardinalDirection::CD_WestSouthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, Nexus::Core::CardinalDirectionNormalized::West))
		{
			return ENCardinalDirection::CD_West;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, Nexus::Core::CardinalDirectionNormalized::WestNorthWest))
		{
			return ENCardinalDirection::CD_WestNorthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, Nexus::Core::CardinalDirectionNormalized::NorthWest))
		{
			return ENCardinalDirection::CD_NorthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, Nexus::Core::CardinalDirectionNormalized::NorthNorthWest))
		{
			return ENCardinalDirection::CD_NorthNorthWest;
		}
		return ENCardinalDirection::CD_North;

	}
	
	static float ToDecimalDegrees(const ENCardinalDirection CardinalDirection)
	{
		switch (CardinalDirection)
		{
			using enum ENCardinalDirection;
		case CD_North:
			return Nexus::Core::CardinalDirection::North;
		case CD_NorthNorthEast:
			return Nexus::Core::CardinalDirection::NorthNorthEast;
		case CD_NorthEast:
			return Nexus::Core::CardinalDirection::NorthEast;
		case CD_EastNorthEast:
			return Nexus::Core::CardinalDirection::EastNorthEast;
		case CD_East:
			return Nexus::Core::CardinalDirection::East;
		case CD_EastSouthEast:
			return Nexus::Core::CardinalDirection::EastSouthEast;
		case CD_SouthEast:
			return Nexus::Core::CardinalDirection::SouthEast;
		case CD_SouthSouthEast:
			return Nexus::Core::CardinalDirection::SouthSouthEast;
		case CD_South:
			return Nexus::Core::CardinalDirection::South;
		case CD_SouthSouthWest:
			return Nexus::Core::CardinalDirection::SouthSouthWest;
		case CD_SouthWest:
			return Nexus::Core::CardinalDirection::SouthWest;
		case CD_WestSouthWest:
			return Nexus::Core::CardinalDirection::WestSouthWest;
		case CD_West:
			return Nexus::Core::CardinalDirection::West;
		case CD_WestNorthWest:
			return Nexus::Core::CardinalDirection::WestNorthWest;
		case CD_NorthWest:
			return Nexus::Core::CardinalDirection::NorthWest;
		case CD_NorthNorthWest:
			return Nexus::Core::CardinalDirection::NorthNorthWest;
		}
		return Nexus::Core::CardinalDirection::North;
	}

	static float ToDecimalDegreesNormalized(const ENCardinalDirection CardinalDirection)
	{
		switch (CardinalDirection)
		{
			using enum ENCardinalDirection;
		case CD_SouthSouthWest:
			return Nexus::Core::CardinalDirectionNormalized::SouthSouthWest;
		case CD_SouthWest:
			return Nexus::Core::CardinalDirectionNormalized::SouthWest;
		case CD_WestSouthWest:
			return Nexus::Core::CardinalDirectionNormalized::WestSouthWest;
		case CD_West:
			return Nexus::Core::CardinalDirectionNormalized::West;
		case CD_WestNorthWest:
			return Nexus::Core::CardinalDirectionNormalized::WestNorthWest;
		case CD_NorthWest:
			return Nexus::Core::CardinalDirectionNormalized::NorthWest;
		case CD_NorthNorthWest:
			return Nexus::Core::CardinalDirectionNormalized::NorthNorthWest;
		case CD_North:
			return Nexus::Core::CardinalDirection::North;
		case CD_NorthNorthEast:
			return Nexus::Core::CardinalDirection::NorthNorthEast;
		case CD_NorthEast:
			return Nexus::Core::CardinalDirection::NorthEast;
		case CD_EastNorthEast:
			return Nexus::Core::CardinalDirection::EastNorthEast;
		case CD_East:
			return Nexus::Core::CardinalDirection::East;
		case CD_EastSouthEast:
			return Nexus::Core::CardinalDirection::EastSouthEast;
		case CD_SouthEast:
			return Nexus::Core::CardinalDirection::SouthEast;
		case CD_SouthSouthEast:
			return Nexus::Core::CardinalDirection::SouthSouthEast;
		case CD_South:
			return Nexus::Core::CardinalDirection::South;

		}
		return Nexus::Core::CardinalDirection::North;
	}
};