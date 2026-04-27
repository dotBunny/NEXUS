// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Types/NRawMesh.h"

class FNWorldContext
{
public:
	
	UWorld* InputWorld;
	TArray<FBoxSphereBounds> InputBounds;
	
	/** Per-element simple-collision meshes gathered from the target world during preprocess, stored in element-local space. */
	TArray<FNRawMesh> CollisionMeshes;
	
	/** World-space transforms corresponding 1:1 with CollisionMeshes. */
	TArray<FTransform> CollisionMeshTransforms;
	
	/** World-space location corresponding 1:1 with CollisionMeshes. */
	TArray<FVector> CollisionMeshLocations;
	
	/** World-space rotation corresponding 1:1 with CollisionMeshes. */
	TArray<FRotator> CollisionMeshRotations;
	
	explicit FNWorldContext(UWorld* TargetWorld, const TArray<FBoxSphereBounds>& TargetBounds)
	{
		InputWorld = TargetWorld;
		InputBounds = TargetBounds;
	}
};
