// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Cell/NCellActor.h"
#include "Developer/NDebugActor.h"
#include "Math/NVectorUtils.h"

class UNOrganComponent;
class ANOrganVolume;
struct FNRawMesh;

/**
 * Grab-bag of ProcGen-specific utilities: cell-level side-car calculators, world/level accessors,
 * and small geometry helpers used by the cell and graph builder.
 */
class NEXUSPROCGEN_API FNProcGenUtils
{
public:
	/** @return Axis-aligned playable bounds of the level, derived from the supplied generation settings. */
	static FBox CalculatePlayableBounds(ULevel* InLevel, const FNCellBoundsGenerationSettings& Settings);

	/** @return Convex hull over the level's qualifying geometry. */
	static FNRawMesh CalculateConvexHull(ULevel* InLevel, const FNCellHullGenerationSettings& Settings);

	/** @return Packed voxel occupancy data for the level. */
	static FNCellVoxelData CalculateVoxelData(ULevel* InLevel, const FNCellVoxelGenerationSettings& Settings);

	/** @return Count of ANCellActor instances in Level. */
	static int32 GetCellActorCountFromLevel(const ULevel* Level);

	/** @return Count of ANCellActor instances in World, optionally ignoring level-instance children. */
	static int32 GetCellActorCountFromWorld(const UWorld* World, bool bIgnoreInstancedLevels = true);

	/** @return First ANCellActor found in Level, or nullptr. */
	static ANCellActor* GetCellActorFromLevel(const ULevel* Level);

	/** @return First ANCellActor found in World, optionally ignoring level-instance children. */
	static ANCellActor* GetCellActorFromWorld(const UWorld* World, bool bIgnoreInstancedLevels = true);

	/**
	 * @param InLevel Level to query.
	 * @param bSorted When true, results are returned in a stable sort order suitable for deterministic traversal.
	 * @return All UNOrganComponent instances attached to actors in Level.
	 */
	static TArray<UNOrganComponent*> GetOrganComponentsFromLevel(const ULevel* InLevel, bool bSorted = true);

	/** @return All ANOrganVolume actors in Level. */
	static TArray<ANOrganVolume*> GetOrganVolumesFromLevel(const ULevel* InLevel);

	/** @return All ANOrganVolume actors across World, optionally ignoring level-instance children. */
	static TArray<ANOrganVolume*> GetOrganVolumesFromWorld(const UWorld* World, bool bIgnoreInstancedLevels = true);

	/** @return InBox rotated by Rotation and translated by Offset. */
	FORCEINLINE static FBox CreateRotatedBox(const FBox& InBox, const FRotator& Rotation, const FVector& Offset)
	{
		return InBox.TransformBy(FTransform(Rotation, Offset));
	}

	/** @return InLocation rotated by Rotation and translated by Offset. */
	FORCEINLINE static FVector OffsetLocation(const FVector& InLocation, const FRotator& Rotation, const FVector& Offset)
	{
		return Rotation.RotateVector(InLocation) + Offset;
	}

	/** @return Units converted to world-size via per-axis UnitSize multipliers. */
	FORCEINLINE static FVector2D GetWorldSize2D(const FIntVector2& Units, const FVector2D& UnitSize)
	{
		return FVector2D(Units.X*UnitSize.X, Units.Y*UnitSize.Y);
	}

	/** @return Units converted to world-size via the XY components of UnitSize. */
	FORCEINLINE static FVector2D GetWorldSize2D(const FIntVector2& Units, const FVector& UnitSize)
	{
		return FVector2D(Units.X*UnitSize.X, Units.Y*UnitSize.Y);
	}

	/** @return Socket corner points (in 2D) for a junction of the given unit dimensions. */
	static TArray<FVector2D> GetSocketPoints2D(const FIntVector2& Units, const FVector2D& UnitSize);

	/** @return Four centered corner points in world space for a rectangle of Width x Height, oriented perpendicular to Axis. */
	static TArray<FVector> GetCenteredWorldCornerPoints2D(const float Width, const float Height, const ENAxis Axis = ENAxis::Z);

	/**
	 * Axis-aligned box ray intersection detection
	 * @remark Slab method (t = (box_coord - ray_origin_coord) / ray_direction_coord)
	 */
	static bool RayAABBIntersection(const FVector& RayOrigin, const FVector& RayDirection, const FBox& Box, FVector& OutIntersectionPoint)
	{
		const FVector InverseDirection = FVector(1.0f / RayDirection.X, 1.0f / RayDirection.Y, 1.0f / RayDirection.Z);
		const FVector t1 = (Box.Min - RayOrigin) * InverseDirection;
		const FVector t2 = (Box.Max - RayOrigin) * InverseDirection;
		const FVector tMin = FVector::Min(t1, t2);
		const FVector tMax = FVector::Max(t1, t2);
		const float tNear = FMath::Max3(tMin.X, tMin.Y, tMin.Z);
		const float tFar = FMath::Min3(tMax.X, tMax.Y, tMax.Z);
		
		if (tNear > tFar || tFar < 0.0f)
		{
			return false;
		}
		
		const float t = (tNear >= 0.0f) ? tNear : tFar;
		OutIntersectionPoint = RayOrigin + RayDirection * t;
	
		return true;
	}
	
	/** Number of sample points emitted by GetVoxelQueryPoints (face + edge + corner neighbours). */
	constexpr static size_t VoxelQueryPointCount = 26;

	/**
	 * Emit the 26 neighbour-sample positions around WorldCenter.
	 * @param VoxelSize Spacing (per-axis) between samples.
	 * @param OutPositions [out] Populated with VoxelQueryPointCount world positions.
	 */
	static void GetVoxelQueryPoints(const FVector& WorldCenter, const FVector& VoxelSize, TArray<FVector>& OutPositions);

	/**
	 * Emit sample endpoints that extend from WorldCenter outward to the faces of LevelBounds.
	 * @param OutPositions [out] Populated with the computed world positions.
	 */
	static void GetVoxelQueryLevelBoundsEndPoints(const FVector& WorldCenter, const FBox& LevelBounds, TArray<FVector>& OutPositions);
};
