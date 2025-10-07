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
	
	static int32 GetCellActorCountFromLevel(const ULevel* Level);
	static int32 GetCellActorCountFromWorld(const UWorld* World, bool bIgnoreInstancedLevels = true);

	static ANCellActor* GetCellActorFromLevel(const ULevel* Level);
	static ANCellActor* GetCellActorFromWorld(const UWorld* World, bool bIgnoreInstancedLevels = true);

	static TArray<UNOrganComponent*> GetOrganComponentsFromLevel(const ULevel* InLevel);
	static TArray<UNOrganComponent*> GetOrganComponentsFromWorld(const UWorld* World, bool bIgnoreInstancedLevels = true);
	
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
	
};
