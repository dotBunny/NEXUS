// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPositionRotation.generated.h"

/**
 * Lightweight pairing of an FVector position and an FRotator rotation.
 *
 * Use when an FTransform is overkill — no scale, no matrix decomposition.
 */
USTRUCT(BlueprintType)
struct FNPositionRotation
{
	GENERATED_BODY()

	/** World-space position. */
	UPROPERTY()
	FVector Position = FVector::ZeroVector;

	/** World-space rotation. */
	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;
};
