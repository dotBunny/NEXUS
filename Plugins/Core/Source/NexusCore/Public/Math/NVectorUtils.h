// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ENAxis : uint8
{
	None = 0,
	X = 1,
	Y = 2,
	Z = 3,
};

/**
 * A collection of utility methods for working with Vectors.
 */
class NEXUSCORE_API FNVectorUtils
{
public:
	FORCEINLINE static FVector TransformPoint(const FVector& Point, const FVector& Origin, const FRotator& Rotation)
	{
		return Origin + Rotation.RotateVector(Point);
	}
	FORCEINLINE static FVector RotateAndOffsetPoint(const FVector& Point, const FRotator& Rotation, const FVector& Offset)
	{
		return Rotation.RotateVector(Point) + Offset;
	}
	static TArray<FVector> RotateAndOffsetPoints(const TArray<FVector>& Points, const FRotator& Rotation, const FVector& Offset);
	static TArray<FVector> RotatePoints(const TArray<FVector>& Vectors, const FRotator& Rotation);
	static TArray<FVector> OffsetPoints(const TArray<FVector>& Vectors, const FVector& Offset);


	static FVector RotatedAroundPivot(const FVector& WorldVector, const FVector& WorldPoint, const FRotator& Rotation)
	{
		return Rotation.RotateVector(WorldVector - WorldPoint) + WorldPoint;
	}
	
	FORCEINLINE static FVector GetClosestGridIntersection(const FVector& Location, const FVector& GridSize)
	{
		return FVector(
			FMath::RoundToInt(Location.X / GridSize.X) * GridSize.X,
			FMath::RoundToInt(Location.Y / GridSize.Y) * GridSize.Y,
			FMath::RoundToInt(Location.Z / GridSize.Z) * GridSize.Z
		);
	}

	FORCEINLINE static FVector GetFurthestGridIntersection(const FVector& Location, const FVector& GridSize)
	{
		return FVector(
		GetCrunchedGridUnit(Location.X, GridSize.X),
		GetCrunchedGridUnit(Location.Y, GridSize.Y),
		GetCrunchedGridUnit(Location.Z, GridSize.Z));
	}
	
	FORCEINLINE static int GetCrunchedGridUnit(const double& Value, const double& Size)
	{
		if (FMath::IsNearlyZero(Value)) return 0;
		
		if (const double RawValue = Value / Size; 
			!FMath::IsNearlyZero(RawValue))
		{
			if (const int CrunchedValue = RawValue; 
				FMath::IsNearlyEqual(RawValue, CrunchedValue))
			{
				return CrunchedValue;
			}
			return FMath::CeilToInt(RawValue);
		}
		return 0;
	}
	
};