// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

/**
 * Identifies a single Cartesian axis (or the absence of one).
 */
UENUM(BlueprintType)
enum class ENAxis : uint8
{
	None = 0,
	X = 1,
	Y = 2,
	Z = 3,
};

/**
 * A collection of utility methods for working with FVectors.
 */
class NEXUSCORE_API FNVectorUtils
{
public:
	/**
	 * Rotates Point by Rotation and offsets the result by Origin.
	 * @param Point Local-space point to transform.
	 * @param Origin Translation applied after the rotation.
	 * @param Rotation Rotation applied before the translation.
	 * @return The transformed world-space point.
	 */
	FORCEINLINE static FVector TransformPoint(const FVector& Point, const FVector& Origin, const FRotator& Rotation)
	{
		return Origin + Rotation.RotateVector(Point);
	}

	/**
	 * Rotates Point by Rotation and adds Offset.
	 * @param Point Local-space point to transform.
	 * @param Rotation Rotation to apply first.
	 * @param Offset Translation applied after the rotation.
	 * @return The transformed point.
	 */
	FORCEINLINE static FVector RotateAndOffsetPoint(const FVector& Point, const FRotator& Rotation, const FVector& Offset)
	{
		return Rotation.RotateVector(Point) + Offset;
	}

	/**
	 * Batch variant of RotateAndOffsetPoint; returns a new array with every input transformed.
	 * @param Points Points to transform.
	 * @param Rotation Rotation applied first.
	 * @param Offset Translation applied after the rotation.
	 * @return A new array of transformed points with the same order as Points.
	 */
	static TArray<FVector> RotateAndOffsetPoints(const TArray<FVector>& Points, const FRotator& Rotation, const FVector& Offset);

	/**
	 * Returns a new array where each vector has been rotated by Rotation.
	 * @param Vectors The points to rotate.
	 * @param Rotation The rotation to apply.
	 */
	static TArray<FVector> RotatePoints(const TArray<FVector>& Vectors, const FRotator& Rotation);

	/**
	 * Returns a new array where each vector has been translated by Offset.
	 * @param Vectors The points to offset.
	 * @param Offset The translation to apply.
	 */
	static TArray<FVector> OffsetPoints(const TArray<FVector>& Vectors, const FVector& Offset);

	/**
	 * Rotates WorldVector around WorldPoint using Rotation as the pivot axis.
	 * @param WorldVector The world-space point being rotated.
	 * @param WorldPoint The pivot around which the rotation occurs.
	 * @param Rotation The rotation to apply relative to the pivot.
	 * @return The rotated point in world space.
	 */
	FORCEINLINE static FVector RotatedAroundPivot(const FVector& WorldVector, const FVector& WorldPoint, const FRotator& Rotation)
	{
		return Rotation.RotateVector(WorldVector - WorldPoint) + WorldPoint;
	}

	/**
	 * Snaps Location to the nearest grid intersection defined by GridSize.
	 * @param Location The point to snap.
	 * @param GridSize Per-axis grid spacing.
	 * @return The nearest grid vertex in each axis.
	 */
	FORCEINLINE static FVector GetClosestGridIntersection(const FVector& Location, const FVector& GridSize)
	{
		return FVector(
			FMath::RoundToInt(Location.X / GridSize.X) * GridSize.X,
			FMath::RoundToInt(Location.Y / GridSize.Y) * GridSize.Y,
			FMath::RoundToInt(Location.Z / GridSize.Z) * GridSize.Z
		);
	}

	/**
	 * Returns the furthest (ceiling-rounded) grid intersection for each axis.
	 * @param Location The point to snap.
	 * @param GridSize Per-axis grid spacing.
	 * @return The grid intersection reached by rounding each axis away from zero toward the outside of its current cell.
	 */
	FORCEINLINE static FVector GetFurthestGridIntersection(const FVector& Location, const FVector& GridSize)
	{
		return FVector(
		GetCrunchedGridUnit(Location.X, GridSize.X),
		GetCrunchedGridUnit(Location.Y, GridSize.Y),
		GetCrunchedGridUnit(Location.Z, GridSize.Z));
	}

	/**
	 * Divides Value by Size and returns the "crunched" unit index, snapping on-grid values down and everything else up.
	 * @param Value The scalar to convert to a grid unit.
	 * @param Size The grid spacing.
	 * @return The integer grid index corresponding to Value.
	 */
	FORCEINLINE static int32 GetCrunchedGridUnit(const double& Value, const double& Size)
	{
		if (FMath::IsNearlyZero(Value)) return 0;

		if (const double RawValue = Value / Size;
			!FMath::IsNearlyZero(RawValue))
		{
			if (const int32 CrunchedValue = RawValue;
				FMath::IsNearlyEqual(RawValue, CrunchedValue))
			{
				return CrunchedValue;
			}
			return FMath::CeilToInt(RawValue);
		}
		return 0;
	}

};