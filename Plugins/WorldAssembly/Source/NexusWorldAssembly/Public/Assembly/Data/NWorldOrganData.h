// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Assembly/Data/NWorldBoneData.h"

class UNOrganComponent;

/**
 * Per-organ derived state computed during FNAssemblyOperationContext::LockAndPreprocess.
 *
 * Holds topological relationships (intersecting/contained organs), the bones that fall inside
 * this organ, and retry budget for the graph-builder stage.
 * 
 * @remarks Unsafe to interact with off of the game-thread.
 */
struct NEXUSWORLDASSEMBLY_API FNWorldOrganData
{
	/** Organ component this context was computed from. */
	TObjectPtr<UNOrganComponent> SourceComponent;

	/** Organs whose bounds intersect but are not fully contained by this one. */
	TArray<TObjectPtr<UNOrganComponent>> IntersectComponents = TArray<TObjectPtr<UNOrganComponent>>();

	/** Organs whose bounds are entirely inside this one. */
	TArray<TObjectPtr<UNOrganComponent>> ContainedComponents = TArray<TObjectPtr<UNOrganComponent>>();

	/** Bones whose transforms fall inside this organ's volume. 
	 * @note If NAssemblyOperationContext's TMap BoneData changes these pointers will dangle. */
	TArray<FNWorldBoneData*> ContainedBones = TArray<FNWorldBoneData*>();

	/** Number of times the graph builder may retry before giving up on this organ. */
	int32 MaximumRetryCount = 3;

	/** Allowed penetration, in world units, between adjacent cell hulls before they are treated as overlapping. */
	float CellHullPenetration = 10.f;

	/** Allowed penetration, in world units, between a placed cell hull and existing world collision before it is rejected. */
	float WorldHullPenetration = 1.f;

	/** Maximum absolute degree deviation (+/-) from a cell's DirectionConstraint heading that still permits placement. */
	float AssemblyDirectionTolerance = 15.f;
	
	/** World-space size of a single voxel, cached from UNWorldAssemblySettings for the graph builder. */
	FVector VoxelSize = FVector(100.f, 100.f, 100.f);

	/** Spatial bounds of the SourceComponent unless bUnbounded. */
	FBoxSphereBounds Bounds;
	
	/** World origin of the SourceComponent's actor. */
	FVector Origin;
};