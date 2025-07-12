﻿// Copyright dotBunny Inc. All Rights Reserved.
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
	
};