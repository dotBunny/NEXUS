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
	
	FORCEINLINE static FVector GetClosestGridLocation(const FVector& Location, const FVector& GridSize,  const FVector& SnapOffset = FVector::ZeroVector)
	{
		const FVector BasePosition = Location - SnapOffset;
		
		const FVector BaseSnappedPosition = FVector(
			FMath::RoundToInt(BasePosition.X / GridSize.X) * GridSize.X,
			FMath::RoundToInt(BasePosition.Y / GridSize.Y) * GridSize.Y,
			FMath::RoundToInt(BasePosition.Z / GridSize.Z) * GridSize.Z
		);

		return BaseSnappedPosition + SnapOffset;
	}
};