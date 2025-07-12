﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Cell/NCellActor.h"
#include "Cell/NCellJunctionDetails.h"

class ANProcGenVolume;
class UNProcGenComponent;
struct FNRawMesh;

class NEXUSPROCGEN_API FNProcGenUtils
{
public:
	static FBox CalculatePlayableBounds(ULevel* InLevel, const FNCellBoundsGenerationSettings& Settings);
	static FNRawMesh CalculateConvexHull(ULevel* InLevel, const FNCellHullGenerationSettings& Settings);
	
	static int GetNCellActorCountFromLevel(const ULevel* Level);
	static int GetNCellActorCountFromWorld(const UWorld* World, bool bIgnoreInstancedLevels = true);

	static ANCellActor* GetNCellActorFromLevel(const ULevel* Level);
	static ANCellActor* GetNCellActorFromWorld(const UWorld* World, bool bIgnoreInstancedLevels = true);

	static TArray<UNProcGenComponent*> GetNProcGenComponentsFromLevel(const ULevel* InLevel);
	static TArray<UNProcGenComponent*> GetNProcGenComponentsFromWorld(const UWorld* World, bool bIgnoreInstancedLevels = true);
	
	static TArray<ANProcGenVolume*> GetNProcGenVolumesFromLevel(const ULevel* InLevel);
	static TArray<ANProcGenVolume*> GetNProcGenVolumesFromWorld(const UWorld* World, bool bIgnoreInstancedLevels = true);
	
	FORCEINLINE static FBox CreateRotatedBox(const FBox& InBox, const FRotator& Rotation, const FVector& Offset)
	{
		return InBox.TransformBy(FTransform(Rotation, Offset));
	}
	
	FORCEINLINE static FVector OffsetLocation(const FVector& InLocation, const FRotator& Rotation, const FVector& Offset)
	{
		return Rotation.RotateVector(InLocation) + Offset;
	}
	
	static FVector2D GetWorldSize(const ENCellJunctionSize& Size);
};
