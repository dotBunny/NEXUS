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
	if (!NavigationSystem->IsInitialized()) \
	{ \
		UE_LOG(LogNexusPicker, Error, TEXT("Attempted to project a generated location(%s) onto a NavMesh prior to the Navigation system having been initialized. Falling back to using the initial location point."), *Location.ToCompactString()) \
		OutLocations.Add(Location); \
		continue; \
	} \
	if (NavigationSystem->ProjectPointToNavigation(Location, NavLocation, FNPickerUtils::DefaultNavExtent, &FNPickerUtils::DefaultNavAgentProperties )) \
	{ \
		if (Location != NavLocation.Location) \
		{ \
			Location = NavLocation.Location; \
		} \
	} \
	else \
	{ \
		UE_LOG(LogNexusPicker, Error, TEXT("Unable to project a location to the NavMesh! Was the location(%s) outside of the NavMesh volume?"), *Location.ToCompactString()); \
	}

/**
 * Utility methods supporting picker operations.
 */
class NEXUSPICKER_API FNPickerUtils
{
public:
	static FCollisionQueryParams DefaultTraceParams;
	static FVector DefaultNavExtent;
	static FNavAgentProperties DefaultNavAgentProperties;
};