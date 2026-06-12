// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include "NPickerMinimal.h"
#include "CoreMinimal.h"
#include "CollisionQueryParams.h"

#define N_PICKER_PARAMS_WORLD_SAFETY \
	UWorld* CachedWorld = Params.CachedWorld.Get(); \
	if (CachedWorld == nullptr) \
	{ \
		UE_LOG(LogNexusPicker, Error, TEXT("A null/non-existent UWorld was provided to Picker.")) \
		return; \
	}

#define N_PICKER_RANDOM_DETERMINISTIC FNMersenneTwister& Random = FNRandom::GetDeterministic();

#define N_PICKER_RANDOM_NONDETERMINISTIC FRandomStream& Random = FNRandom::GetNonDeterministic();

#define N_PICKER_PROJECTION_TRACE_PREFIX \
	FHitResult HitResult(ForceInit);
#define N_PICKER_PROJECTION_TRACE \
	if (CachedWorld->LineTraceSingleByChannel(HitResult, Location, (Location + Params.Projection), Params.CollisionChannel, FNPickerUtils::CollisionQueryParams)) \
	{ \
		Location = HitResult.Location; \
	}

#define N_PICKER_PROJECTION_NAVMESH_V1_PREFIX \
	UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(CachedWorld); \
	FNavLocation NavLocation;
#define N_PICKER_PROJECTION_NAVMESH_V1 \
	if (NavigationSystem != nullptr) \
	{ \
		if (!NavigationSystem->IsInitialized()) \
		{ \
			UE_LOG(LogNexusPicker, Error, TEXT("Attempted to project a generated location(%s) onto a NavMesh prior to the Navigation system having been initialized. Falling back to using the initial location point."), *Location.ToCompactString()) \
			OutLocations.Add(Location); \
			continue; \
		} \
		if (NavigationSystem->ProjectPointToNavigation(Location, NavLocation, FNPickerUtils::NavQueryExtent, &FNPickerUtils::NavAgentProperties )) \
		{ \
			if (Location != NavLocation.Location) \
			{ \
				Location = NavLocation.Location; \
			} \
		} \
		else \
		{ \
			UE_LOG(LogNexusPicker, Error, TEXT("Unable to project a location to the NavMesh! Was the location(%s) outside of the NavMesh volume, or inside of a NavModifier. Adjust your NavQueryExtent to account for this increased distance, or ensure your dimensions are within the NavMesh volume."), *Location.ToCompactString()); \
		} \
	} \
	else \
	{ \
		UE_LOG(LogNexusPicker, Error, TEXT("Unable to project a location to the NavMesh as NavigationSystemV1 was null.")); \
	}

/**
 * Shared picker configuration defaults used by all pickers when resolving a generated point onto geometry or navmesh via ENPickerProjectionMode.
 */
class NEXUSPICKER_API FNPickerUtils
{
public:
	/** Collision query parameters used by the trace-based projection path. 
	 * @note Should only be accessed from the Game-thread.
	 */
	static FCollisionQueryParams CollisionQueryParams;
	/** Query extent used when projecting onto the navmesh; adjust if your navmesh tolerance needs widening. 
	 * @note Should only be accessed from the Game-thread.
	 */
	static FVector NavQueryExtent;
	/** Nav agent properties used when resolving a navmesh location for a generated point.
	 * @note Should only be accessed from the Game-thread. 
	 */
	static FNavAgentProperties NavAgentProperties;
};