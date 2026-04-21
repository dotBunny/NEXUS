// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Organ/NBoneComponent.h"

/**
 * Per-bone derived state cached during operation preprocessing.
 *
 * Stores the bone component and its computed world-space corner points so downstream
 * stages don't recompute them per access.
 */
struct NEXUSPROCGEN_API FNProcGenOperationBoneContext
{
	/** Bone component this context was computed from. */
	UNBoneComponent* SourceComponent;

	/** Cached world-space corners of the bone's junction socket. */
	TArray<FVector> CornerPoints;
};