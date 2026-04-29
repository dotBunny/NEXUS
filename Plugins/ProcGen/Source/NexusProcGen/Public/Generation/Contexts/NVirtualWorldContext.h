// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/Graph/NProcGenGraphCellNode.h"
#include "Types/NRawMesh.h"

class FNVirtualWorldContext
{
public:
	
	UWorld* InputWorld;
	TArray<FBoxSphereBounds> InputBounds;
	
	/** Per-element simple-collision meshes gathered from the target world during preprocess, stored in element-local space. */
	TArray<FNRawMesh> WorldCollisionMeshes;
	
	/** World-space transforms corresponding 1:1 with WorldCollisionMeshes. */
	TArray<FTransform> WorldCollisionMeshTransforms;
	
	/** World-space location corresponding 1:1 with WorldCollisionMeshes. */
	TArray<FVector> WorldCollisionMeshLocations;
	
	/** World-space rotation corresponding 1:1 with WorldCollisionMeshes. */
	TArray<FRotator> WorldCollisionMeshRotations;
	
	
	TArray<FNProcGenGraphCellNode*> NodeIndex;
	TArray<FNRawMesh> NodeCollisionMeshes;
	TArray<FVector> NodeCollisionMeshLocations;
	TArray<FRotator> NodeCollisionMeshRotations;
	
	explicit FNVirtualWorldContext(UWorld* TargetWorld, const TArray<FBoxSphereBounds>& TargetBounds)
	{
		InputWorld = TargetWorld;
		InputBounds = TargetBounds;
	}
};
