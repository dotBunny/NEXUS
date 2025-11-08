// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Cell/NCellActor.h"
#include "Math/NVectorUtils.h"

class UNOrganComponent;
class ANOrganVolume;
struct FNRawMesh;

class NEXUSPROCGEN_API FNProcGenUtils
{
public:
	static FBox CalculatePlayableBounds(ULevel* InLevel, const FNCellBoundsGenerationSettings& Settings);
	static FNRawMesh CalculateConvexHull(ULevel* InLevel, const FNCellHullGenerationSettings& Settings);
	static FNCellVoxelData CalculateVoxelData(ULevel* InLevel, const FNCellVoxelGenerationSettings& Settings);
	
	static int32 GetCellActorCountFromLevel(const ULevel* Level);
	static int32 GetCellActorCountFromWorld(const UWorld* World, bool bIgnoreInstancedLevels = true);

	static ANCellActor* GetCellActorFromLevel(const ULevel* Level);
	static ANCellActor* GetCellActorFromWorld(const UWorld* World, bool bIgnoreInstancedLevels = true);

	static TArray<UNOrganComponent*> GetOrganComponentsFromLevel(const ULevel* InLevel);
	static TArray<ANOrganVolume*> GetOrganVolumesFromLevel(const ULevel* InLevel);
	static TArray<ANOrganVolume*> GetOrganVolumesFromWorld(const UWorld* World, bool bIgnoreInstancedLevels = true);
	
	FORCEINLINE static FBox CreateRotatedBox(const FBox& InBox, const FRotator& Rotation, const FVector& Offset)
	{
		return InBox.TransformBy(FTransform(Rotation, Offset));
	}
	
	FORCEINLINE static FVector OffsetLocation(const FVector& InLocation, const FRotator& Rotation, const FVector& Offset)
	{
		return Rotation.RotateVector(InLocation) + Offset;
	}

	FORCEINLINE static FVector2D GetWorldSize2D(const FIntVector2& Units, const FVector& UnitSize)
	{
		return FVector2D(Units.X*UnitSize.X, Units.Y*UnitSize.Y);
	}

	static TArray<FVector2D> GetCenteredWorldPoints2D(const FIntVector2& Units, const FVector& UnitSize);

	static TArray<FVector> GetCenteredWorldCornerPoints2D(const FVector& WorldCenter, const FRotator& Rotation, const float Width, const float Height, const ENAxis Axis = ENAxis::Z);

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
	
	constexpr static size_t RayTestNumber = 26;
	static void GetVoxelTestRayEndPoints(const FVector& WorldCenter, const FBox& LevelBounds, TArray<FVector>& OutPositions);
};
