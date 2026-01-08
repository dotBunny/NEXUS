// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#define N_CARDINAL_NORTH 0.0f
#define N_CARDINAL_NORTH_NORTH_EAST 22.5f
#define N_CARDINAL_NORTH_EAST 45.0f
#define N_CARDINAL_EAST_NORTH_EAST 67.5f
#define N_CARDINAL_EAST 90.0f
#define N_CARDINAL_EAST_SOUTH_EAST 112.5f
#define N_CARDINAL_SOUTH_EAST 135.0f
#define N_CARDINAL_SOUTH_SOUTH_EAST 157.5f
#define N_CARDINAL_SOUTH 180.0f

#define N_CARDINAL_SOUTH_SOUTH_WEST 202.5f
#define N_CARDINAL_SOUTH_WEST 225.0f
#define N_CARDINAL_WEST_SOUTH_WEST 247.5f
#define N_CARDINAL_WEST 270.0f
#define N_CARDINAL_WEST_NORTH_WEST 292.5f
#define N_CARDINAL_NORTH_WEST 315.0f
#define N_CARDINAL_NORTH_NORTH_WEST 337.5f

#define N_CARDINAL_SOUTH_SOUTH_WEST_NORMALIZED -157.5f
#define N_CARDINAL_SOUTH_WEST_NORMALIZED -135.0f
#define N_CARDINAL_WEST_SOUTH_WEST_NORMALIZED -112.5f
#define N_CARDINAL_WEST_NORMALIZED -90.0f
#define N_CARDINAL_WEST_NORTH_WEST_NORMALIZED -67.5f
#define N_CARDINAL_NORTH_WEST_NORMALIZED -45.0f
#define N_CARDINAL_NORTH_NORTH_WEST_NORMALIZED -22.5f


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
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_NORTH))
		{
			return ENCardinalDirection::CD_North;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_NORTH_NORTH_EAST))
		{
			return ENCardinalDirection::CD_NorthNorthEast;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_NORTH_EAST))
		{
			return ENCardinalDirection::CD_NorthEast;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_EAST_NORTH_EAST))
		{
			return ENCardinalDirection::CD_EastNorthEast;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_EAST))
		{
			return ENCardinalDirection::CD_East;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_EAST_SOUTH_EAST))
		{
			return ENCardinalDirection::CD_EastSouthEast;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_SOUTH_EAST))
		{
			return ENCardinalDirection::CD_SouthEast;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_SOUTH_SOUTH_EAST))
		{
			return ENCardinalDirection::CD_SouthSouthEast;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_SOUTH))
		{
			return ENCardinalDirection::CD_South;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_SOUTH_SOUTH_WEST))
		{
			return ENCardinalDirection::CD_SouthSouthWest;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_SOUTH_WEST))
		{
			return ENCardinalDirection::CD_SouthWest;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_WEST_SOUTH_WEST))
		{
			return ENCardinalDirection::CD_WestSouthWest;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_WEST))
		{
			return ENCardinalDirection::CD_West;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_WEST_NORTH_WEST))
		{
			return ENCardinalDirection::CD_WestNorthWest;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_NORTH_WEST))
		{
			return ENCardinalDirection::CD_NorthWest;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_NORTH_NORTH_WEST))
		{
			return ENCardinalDirection::CD_NorthNorthWest;
		}
		return ENCardinalDirection::CD_North;
	}

	static ENCardinalDirection ToCardinalDirectionNormalized(const float NormalizedAngle)
	{
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_NORTH))
		{
			return ENCardinalDirection::CD_North;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_NORTH_NORTH_EAST))
		{
			return ENCardinalDirection::CD_NorthNorthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_NORTH_EAST))
		{
			return ENCardinalDirection::CD_NorthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_EAST_NORTH_EAST))
		{
			return ENCardinalDirection::CD_EastNorthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_EAST))
		{
			return ENCardinalDirection::CD_East;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_EAST_SOUTH_EAST))
		{
			return ENCardinalDirection::CD_EastSouthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_SOUTH_EAST))
		{
			return ENCardinalDirection::CD_SouthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_SOUTH_SOUTH_EAST))
		{
			return ENCardinalDirection::CD_SouthSouthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_SOUTH) || FMath::IsNearlyEqual(NormalizedAngle, -N_CARDINAL_SOUTH))
		{
			return ENCardinalDirection::CD_South;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_SOUTH_SOUTH_WEST_NORMALIZED))
		{
			return ENCardinalDirection::CD_SouthSouthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_SOUTH_WEST_NORMALIZED))
		{
			return ENCardinalDirection::CD_SouthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_WEST_SOUTH_WEST_NORMALIZED))
		{
			return ENCardinalDirection::CD_WestSouthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_WEST_NORMALIZED))
		{
			return ENCardinalDirection::CD_West;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_WEST_NORTH_WEST_NORMALIZED))
		{
			return ENCardinalDirection::CD_WestNorthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_NORTH_WEST_NORMALIZED))
		{
			return ENCardinalDirection::CD_NorthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_NORTH_NORTH_WEST_NORMALIZED))
		{
			return ENCardinalDirection::CD_NorthNorthWest;
		}
		return ENCardinalDirection::CD_North;

	}
	
	static float ToDecimalDegrees(const ENCardinalDirection CardinalDirection)
	{
		switch (CardinalDirection)
		{
		case ENCardinalDirection::CD_North:
			return N_CARDINAL_NORTH;
		case ENCardinalDirection::CD_NorthNorthEast:
			return N_CARDINAL_NORTH_NORTH_EAST;
		case ENCardinalDirection::CD_NorthEast:
			return N_CARDINAL_NORTH_EAST;
		case ENCardinalDirection::CD_EastNorthEast:
			return N_CARDINAL_EAST_NORTH_EAST;
		case ENCardinalDirection::CD_East:
			return N_CARDINAL_EAST;
		case ENCardinalDirection::CD_EastSouthEast:
			return N_CARDINAL_EAST_SOUTH_EAST;
		case ENCardinalDirection::CD_SouthEast:
			return N_CARDINAL_SOUTH_EAST;
		case ENCardinalDirection::CD_SouthSouthEast:
			return N_CARDINAL_SOUTH_SOUTH_EAST;
		case ENCardinalDirection::CD_South:
			return N_CARDINAL_SOUTH;
		case ENCardinalDirection::CD_SouthSouthWest:
			return N_CARDINAL_SOUTH_SOUTH_WEST;
		case ENCardinalDirection::CD_SouthWest:
			return N_CARDINAL_SOUTH_WEST;
		case ENCardinalDirection::CD_WestSouthWest:
			return N_CARDINAL_WEST_SOUTH_WEST;
		case ENCardinalDirection::CD_West:
			return N_CARDINAL_WEST;
		case ENCardinalDirection::CD_WestNorthWest:
			return N_CARDINAL_WEST_NORTH_WEST;
		case ENCardinalDirection::CD_NorthWest:
			return N_CARDINAL_NORTH_WEST;
		case ENCardinalDirection::CD_NorthNorthWest:
			return N_CARDINAL_NORTH_NORTH_WEST;
		}
		return N_CARDINAL_NORTH
;
	}

	static float ToDecimalDegreesNormalized(const ENCardinalDirection CardinalDirection)
	{
		switch (CardinalDirection)
		{
		case ENCardinalDirection::CD_SouthSouthWest:
			return N_CARDINAL_SOUTH_SOUTH_WEST_NORMALIZED;			
		case ENCardinalDirection::CD_SouthWest:
			return N_CARDINAL_SOUTH_WEST_NORMALIZED;
		case ENCardinalDirection::CD_WestSouthWest:
			return N_CARDINAL_WEST_SOUTH_WEST_NORMALIZED;
		case ENCardinalDirection::CD_West:
			return N_CARDINAL_WEST_NORMALIZED;
		case ENCardinalDirection::CD_WestNorthWest:
			return N_CARDINAL_WEST_NORTH_WEST_NORMALIZED;
		case ENCardinalDirection::CD_NorthWest:
			return N_CARDINAL_NORTH_WEST_NORMALIZED;
		case ENCardinalDirection::CD_NorthNorthWest:
			return N_CARDINAL_NORTH_NORTH_WEST_NORMALIZED;
		case ENCardinalDirection::CD_North:
			return N_CARDINAL_NORTH;
		case ENCardinalDirection::CD_NorthNorthEast:
			return N_CARDINAL_NORTH_NORTH_EAST;
		case ENCardinalDirection::CD_NorthEast:
			return N_CARDINAL_NORTH_EAST;
		case ENCardinalDirection::CD_EastNorthEast:
			return N_CARDINAL_EAST_NORTH_EAST;
		case ENCardinalDirection::CD_East:
			return N_CARDINAL_EAST;
		case ENCardinalDirection::CD_EastSouthEast:
			return N_CARDINAL_EAST_SOUTH_EAST;
		case ENCardinalDirection::CD_SouthEast:
			return N_CARDINAL_SOUTH_EAST;
		case ENCardinalDirection::CD_SouthSouthEast:
			return N_CARDINAL_SOUTH_SOUTH_EAST;
		case ENCardinalDirection::CD_South:
			return N_CARDINAL_SOUTH;

		}
		return N_CARDINAL_NORTH;
	}
};