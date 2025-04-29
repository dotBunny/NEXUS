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
	NCD_North					UMETA(DisplayName = "North"),
	NCD_NorthNorthEast			UMETA(DisplayName = "North North East"),
	NCD_NorthEast				UMETA(DisplayName = "North East"),
	NCD_EastNorthEast			UMETA(DisplayName = "East North East"),
	NCD_East					UMETA(DisplayName = "East"),
	NCD_EastSouthEast			UMETA(DisplayName = "East South East"),
	NCD_SouthEast				UMETA(DisplayName = "South East"),
	NCD_SouthSouthEast			UMETA(DisplayName = "South South East"),
	NCD_South					UMETA(DisplayName = "South"),
	NCD_SouthSouthWest			UMETA(DisplayName = "South South West"),
	NCD_SouthWest				UMETA(DisplayName = "South West"),
	NCD_WestSouthWest			UMETA(DisplayName = "West South West"),
	NCD_West					UMETA(DisplayName = "West"),
	NCD_WestNorthWest 			UMETA(DisplayName = "West North West"),
	NCD_NorthWest 				UMETA(DisplayName = "North West"),
	NCD_NorthNorthWest 			UMETA(DisplayName = "North North West"),
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
			return ENCardinalDirection::NCD_North;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_NORTH_NORTH_EAST))
		{
			return ENCardinalDirection::NCD_NorthNorthEast;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_NORTH_EAST))
		{
			return ENCardinalDirection::NCD_NorthEast;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_EAST_NORTH_EAST))
		{
			return ENCardinalDirection::NCD_EastNorthEast;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_EAST))
		{
			return ENCardinalDirection::NCD_East;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_EAST_SOUTH_EAST))
		{
			return ENCardinalDirection::NCD_EastSouthEast;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_SOUTH_EAST))
		{
			return ENCardinalDirection::NCD_SouthEast;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_SOUTH_SOUTH_EAST))
		{
			return ENCardinalDirection::NCD_SouthSouthEast;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_SOUTH))
		{
			return ENCardinalDirection::NCD_South;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_SOUTH_SOUTH_WEST))
		{
			return ENCardinalDirection::NCD_SouthSouthWest;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_SOUTH_WEST))
		{
			return ENCardinalDirection::NCD_SouthWest;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_WEST_SOUTH_WEST))
		{
			return ENCardinalDirection::NCD_WestSouthWest;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_WEST))
		{
			return ENCardinalDirection::NCD_West;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_WEST_NORTH_WEST))
		{
			return ENCardinalDirection::NCD_WestNorthWest;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_NORTH_WEST))
		{
			return ENCardinalDirection::NCD_NorthWest;
		}
		if (FMath::IsNearlyEqual(Angle, N_CARDINAL_NORTH_NORTH_WEST))
		{
			return ENCardinalDirection::NCD_NorthNorthWest;
		}
		return ENCardinalDirection::NCD_North;
	}

	static ENCardinalDirection ToCardinalDirectionNormalized(const float NormalizedAngle)
	{
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_NORTH))
		{
			return ENCardinalDirection::NCD_North;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_NORTH_NORTH_EAST))
		{
			return ENCardinalDirection::NCD_NorthNorthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_NORTH_EAST))
		{
			return ENCardinalDirection::NCD_NorthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_EAST_NORTH_EAST))
		{
			return ENCardinalDirection::NCD_EastNorthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_EAST))
		{
			return ENCardinalDirection::NCD_East;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_EAST_SOUTH_EAST))
		{
			return ENCardinalDirection::NCD_EastSouthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_SOUTH_EAST))
		{
			return ENCardinalDirection::NCD_SouthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_SOUTH_SOUTH_EAST))
		{
			return ENCardinalDirection::NCD_SouthSouthEast;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_SOUTH) || FMath::IsNearlyEqual(NormalizedAngle, -N_CARDINAL_SOUTH))
		{
			return ENCardinalDirection::NCD_South;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_SOUTH_SOUTH_WEST_NORMALIZED))
		{
			return ENCardinalDirection::NCD_SouthSouthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_SOUTH_WEST_NORMALIZED))
		{
			return ENCardinalDirection::NCD_SouthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_WEST_SOUTH_WEST_NORMALIZED))
		{
			return ENCardinalDirection::NCD_WestSouthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_WEST_NORMALIZED))
		{
			return ENCardinalDirection::NCD_West;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_WEST_NORTH_WEST_NORMALIZED))
		{
			return ENCardinalDirection::NCD_WestNorthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_NORTH_WEST_NORMALIZED))
		{
			return ENCardinalDirection::NCD_NorthWest;
		}
		if (FMath::IsNearlyEqual(NormalizedAngle, N_CARDINAL_NORTH_NORTH_WEST_NORMALIZED))
		{
			return ENCardinalDirection::NCD_NorthNorthWest;
		}
		return ENCardinalDirection::NCD_North;

	}
	
	static float ToDecimalDegrees(const ENCardinalDirection CardinalDirection)
	{
		switch (CardinalDirection)
		{
		case ENCardinalDirection::NCD_North:
			return N_CARDINAL_NORTH;
		case ENCardinalDirection::NCD_NorthNorthEast:
			return N_CARDINAL_NORTH_NORTH_EAST;
		case ENCardinalDirection::NCD_NorthEast:
			return N_CARDINAL_NORTH_EAST;
		case ENCardinalDirection::NCD_EastNorthEast:
			return N_CARDINAL_EAST_NORTH_EAST;
		case ENCardinalDirection::NCD_East:
			return N_CARDINAL_EAST;
		case ENCardinalDirection::NCD_EastSouthEast:
			return N_CARDINAL_EAST_SOUTH_EAST;
		case ENCardinalDirection::NCD_SouthEast:
			return N_CARDINAL_SOUTH_EAST;
		case ENCardinalDirection::NCD_SouthSouthEast:
			return N_CARDINAL_SOUTH_SOUTH_EAST;
		case ENCardinalDirection::NCD_South:
			return N_CARDINAL_SOUTH;
		case ENCardinalDirection::NCD_SouthSouthWest:
			return N_CARDINAL_SOUTH_SOUTH_WEST;
		case ENCardinalDirection::NCD_SouthWest:
			return N_CARDINAL_SOUTH_WEST;
		case ENCardinalDirection::NCD_WestSouthWest:
			return N_CARDINAL_WEST_SOUTH_WEST;
		case ENCardinalDirection::NCD_West:
			return N_CARDINAL_WEST;
		case ENCardinalDirection::NCD_WestNorthWest:
			return N_CARDINAL_WEST_NORTH_WEST;
		case ENCardinalDirection::NCD_NorthWest:
			return N_CARDINAL_NORTH_WEST;
		case ENCardinalDirection::NCD_NorthNorthWest:
			return N_CARDINAL_NORTH_NORTH_WEST;
		}
		return N_CARDINAL_NORTH
;
	}

	static float ToDecimalDegreesNormalized(const ENCardinalDirection CardinalDirection)
	{
		switch (CardinalDirection)
		{
		case ENCardinalDirection::NCD_SouthSouthWest:
			return N_CARDINAL_SOUTH_SOUTH_WEST_NORMALIZED;			
		case ENCardinalDirection::NCD_SouthWest:
			return N_CARDINAL_SOUTH_WEST_NORMALIZED;
		case ENCardinalDirection::NCD_WestSouthWest:
			return N_CARDINAL_WEST_SOUTH_WEST_NORMALIZED;
		case ENCardinalDirection::NCD_West:
			return N_CARDINAL_WEST_NORMALIZED;
		case ENCardinalDirection::NCD_WestNorthWest:
			return N_CARDINAL_WEST_NORTH_WEST_NORMALIZED;
		case ENCardinalDirection::NCD_NorthWest:
			return N_CARDINAL_NORTH_WEST_NORMALIZED;
		case ENCardinalDirection::NCD_NorthNorthWest:
			return N_CARDINAL_NORTH_NORTH_WEST_NORMALIZED;
		case ENCardinalDirection::NCD_North:
			return N_CARDINAL_NORTH;
		case ENCardinalDirection::NCD_NorthNorthEast:
			return N_CARDINAL_NORTH_NORTH_EAST;
		case ENCardinalDirection::NCD_NorthEast:
			return N_CARDINAL_NORTH_EAST;
		case ENCardinalDirection::NCD_EastNorthEast:
			return N_CARDINAL_EAST_NORTH_EAST;
		case ENCardinalDirection::NCD_East:
			return N_CARDINAL_EAST;
		case ENCardinalDirection::NCD_EastSouthEast:
			return N_CARDINAL_EAST_SOUTH_EAST;
		case ENCardinalDirection::NCD_SouthEast:
			return N_CARDINAL_SOUTH_EAST;
		case ENCardinalDirection::NCD_SouthSouthEast:
			return N_CARDINAL_SOUTH_SOUTH_EAST;
		case ENCardinalDirection::NCD_South:
			return N_CARDINAL_SOUTH;

		}
		return N_CARDINAL_NORTH;
	}
};