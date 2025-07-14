// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "CollisionQueryParams.h"

#define N_VARIABLES_PICKER_GROUNDED() \
	const UWorld* InWorld = nullptr, const FVector& CastBuffer = FNPickerUtils::DefaultCastBuffer, const ECollisionChannel CollisionChannel = ECollisionChannel::ECC_WorldStatic
#define N_IMPLEMENT_PICKER_GROUNDED() \
	FHitResult HitResult(ForceInit); \
	if (InWorld->LineTraceSingleByChannel(HitResult, OutLocation, (OutLocation + CastBuffer), CollisionChannel, FNPickerUtils::DefaultTraceParams)) \
	{ \
		OutLocation = HitResult.Location; \
	}
#define N_GROUNDING_TOLERANCE 0.0001f

/**
 * Utility methods supporting picker operations.
 */
class NEXUSPICKER_API FNPickerUtils
{
public:
	static FCollisionQueryParams DefaultTraceParams;
	static FVector DefaultCastBuffer;
};