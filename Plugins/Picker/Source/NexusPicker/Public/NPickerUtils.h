// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include "NPickerMinimal.h"
#include "CoreMinimal.h"
#include "CollisionQueryParams.h"

#define N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX \
	FHitResult HitResult(ForceInit);
#define N_IMPLEMENT_PICKER_PROJECTION_TRACE \
	if (Params.CachedWorld->LineTraceSingleByChannel(HitResult, Location, (Location + Params.Projection), Params.CollisionChannel, FNPickerUtils::DefaultTraceParams)) \
	{ \
		Location = HitResult.Location; \
	}

#define N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX \
	UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Params.CachedWorld); \
	FNavLocation NavLocation;
#define N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1 \
	NavigationSystem->ProjectPointToNavigation(Location, NavLocation); \
	if (Location != NavLocation.Location) \
	{ \
		Location = NavLocation.Location; \
	}

/**
 * Utility methods supporting picker operations.
 */
class NEXUSPICKER_API FNPickerUtils
{
public:
	static FCollisionQueryParams DefaultTraceParams;
	
	static FVector DefaultProjection;
	static FRotator DefaultRotation;

	static FRotator BaseRotation;
	static FMatrix BaseMatrix;
};