// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Types/NRawMesh.h"

class FNCreateWorldContext
{
public:
	
	UWorld* World;
	
	/** Per-element simple-collision meshes gathered from the target world during preprocess, stored in element-local space. */
	TArray<FNRawMesh> CollisionMeshes;

	/** World-space transforms corresponding 1:1 with CollisionMeshes. */
	TArray<FTransform> CollisionMeshTransforms;
	
	TArray<FBoxSphereBounds> Bounds;
	
	explicit FNCreateWorldContext(UWorld* TargetWorld, const TArray<FBoxSphereBounds>& TargetBounds)
	{
		World = TargetWorld;
		Bounds = TargetBounds;
	}
};
