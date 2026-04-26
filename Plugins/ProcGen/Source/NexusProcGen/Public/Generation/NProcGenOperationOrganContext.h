// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NProcGenOperationBoneContext.h"

class UNOrganComponent;

/**
 * Per-organ derived state computed during FNProcGenOperationContext::LockAndPreprocess.
 *
 * Holds topological relationships (intersecting/contained organs), the bones that fall inside
 * this organ, and retry budget for the graph-builder stage.
 */
struct NEXUSPROCGEN_API FNProcGenOperationOrganContext
{
	/** Organ component this context was computed from. */
	UNOrganComponent* SourceComponent;

	/** Organs whose bounds intersect but are not fully contained by this one. */
	TArray<UNOrganComponent*> IntersectComponents = TArray<UNOrganComponent*>();

	/** Organs whose bounds are entirely inside this one. */
	TArray<UNOrganComponent*> ContainedComponents = TArray<UNOrganComponent*>();

	/** Bones whose transforms fall inside this organ's volume. */
	TArray<FNProcGenOperationBoneContext*> ContainedBones = TArray<FNProcGenOperationBoneContext*>();

	/** Number of times the graph builder may retry before giving up on this organ. */
	int32 MaximumRetryCount = 3;
	
	/** Spatial bounds of the SourceComponent unless bUnbounded. */
	FBoxSphereBounds Bounds;
	
	/** World origin of the SourceComponent's actor. */
	FVector Origin;
};