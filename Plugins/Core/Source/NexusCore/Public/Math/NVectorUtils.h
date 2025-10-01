// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum ENAxis : uint8
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
	FORCEINLINE static FVector RotateAndOffsetVector(const FVector& Vector, const FRotator& Rotation, const FVector& Offset)
	{
		return Rotation.RotateVector(Vector) + Offset;
	}
	static TArray<FVector> RotateAndOffsetVectors(const TArray<FVector>& Vectors, const FRotator& Rotation, const FVector& Offset);
	static TArray<FVector> RotateVectors(const TArray<FVector>& Vectors, const FRotator& Rotation);
	static TArray<FVector> OffsetVectors(const TArray<FVector>& Vectors, const FVector& Offset);


	static FVector RotatedAroundPivot(const FVector& WorldVector, const FVector& WorldPoint, const FRotator& Rotation)
	{
		return Rotation.RotateVector(WorldVector - WorldPoint) + WorldPoint;
	}
	
	FORCEINLINE static bool TrySnapToGrid(const FVector& Location, FVector& OutLocation, const FVector& GridSize,
		const float SnapDistanceSquared = 10000.f, const FVector& SnapOffset = FVector::ZeroVector)
	{
		// Assign default output (unsnapped)
		OutLocation = Location;

		// Remove any offset prior to calculations
		const FVector BasePosition = Location - SnapOffset;
		
		const FVector BaseSnappedPosition = FVector(
			FMath::RoundToInt(BasePosition.X / GridSize.X) * GridSize.X,
			FMath::RoundToInt(BasePosition.Y / GridSize.Y) * GridSize.Y,
			FMath::RoundToInt(BasePosition.Z / GridSize.Z) * GridSize.Z
		);
		
		FVector SnappedDelta = FVector(
			Location.X - BaseSnappedPosition.X,
			Location.Y - BaseSnappedPosition.Y,
			Location.Z - BaseSnappedPosition.Z);

		UE_LOG(LogTemp, Warning, TEXT("SnappedDelta: %s"), *SnappedDelta.ToString())
		
		
		// if (FVector::DistSquared(BaseSnappedPosition, Location) < SnapDistanceSquared)
		// {
		// 	UE_LOG(LogTemp, Log, TEXT("SnapToGrid: %f :  %s -> %s"),
		// 		FVector::DistSquared(Location, FinalPosition), *Location.ToString(), *FinalPosition.ToString());
		// 	OutLocation = FinalPosition;
		// 	return true;
		// }
		//
		return false;
	}
};