// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "NPickerParams.h"
#include "NPickerSettings.h"
// ON_SCOPE_EXIT is used by the navmesh projection path.
#include "Misc/ScopeExit.h"

/**
 * Shared point-emission scaffolding used by every picker.
 *
 * Each picker only differs in how it produces a single point; the surrounding "generate N points and
 * resolve them onto geometry/navmesh" loop is identical everywhere. Hoisting it here keeps the projection
 * behavior (and the subtle navmesh failure handling) in exactly one place rather than copied per picker.
 */
class FNPickerProjection
{
public:
	/**
	 * Generates Params.Count points, resolving the configured projection mode a single time before the loop.
	 * @param OutLocations Destination array; generated (and projected) points are appended.
	 * @param CachedWorld Resolved world used for trace/navmesh projection; pass nullptr to force passthrough.
	 * @param Params Picker parameters supplying Count and the projection settings.
	 * @param GeneratePoint Callable invoked once per point, returning the pre-projection world-space location.
	 * @note Game-thread only; mirrors the per-picker RNG threading constraints.
	 */
	template <typename FGeneratePoint>
	static void Emit(TArray<FVector>& OutLocations, UWorld* CachedWorld, const FNPickerParams& Params, FGeneratePoint&& GeneratePoint)
	{
		if (Params.ProjectionMode != ENPickerProjectionMode::None && CachedWorld == nullptr)
		{
			UE_LOG(LogNexusPicker, Warning, TEXT("A ProjectionMode is set but no world is available (CachedWorld is null); points will be returned unprojected. Ensure a valid WorldContext is supplied, or set Params.CachedWorld before generating points."));
		}
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && CachedWorld != nullptr)
		{
			const FCollisionQueryParams CollisionQueryParams = UNPickerSettings::Get()->MakeCollisionQueryParams();
			FHitResult HitResult(ForceInit);
			for (int32 i = 0; i < Params.Count; i++)
			{
				FVector Location = GeneratePoint();
				if (CachedWorld->LineTraceSingleByChannel(HitResult, Location, (Location + Params.Projection), Params.CollisionChannel, CollisionQueryParams))
				{
					Location = HitResult.Location;
				}
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && CachedWorld != nullptr)
		{
			const UNPickerSettings* Settings = UNPickerSettings::Get();
			const FVector NavQueryExtent = Settings->NavQueryExtent;
			const FNavAgentProperties NavAgentProperties(Settings->NavAgentRadius, Settings->NavAgentHeight);
			UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(CachedWorld);
			FNavLocation NavLocation;
			int32 NavMeshProjectionFailures = 0;
			ON_SCOPE_EXIT
			{
				if (NavMeshProjectionFailures > 0)
				{
					UE_LOG(LogNexusPicker, Warning, TEXT("Unable to project %d generated location(s) to the NavMesh. Were they outside the NavMesh volume, or inside a NavModifier? Adjust your NavQueryExtent to account for the increased distance, or ensure your dimensions are within the NavMesh volume. Enable Verbose logging for LogNexusPicker to see each failed location."), NavMeshProjectionFailures);
				}
			};
			for (int32 i = 0; i < Params.Count; i++)
			{
				FVector Location = GeneratePoint();
				if (NavigationSystem != nullptr)
				{
					if (!NavigationSystem->IsInitialized())
					{
						UE_LOG(LogNexusPicker, Error, TEXT("Attempted to project a generated location(%s) onto a NavMesh prior to the Navigation system having been initialized. Falling back to using the initial location point."), *Location.ToCompactString())
						OutLocations.Add(Location);
						continue;
					}
					if (NavigationSystem->ProjectPointToNavigation(Location, NavLocation, NavQueryExtent, &NavAgentProperties))
					{
						if (Location != NavLocation.Location)
						{
							Location = NavLocation.Location;
						}
					}
					else
					{
						UE_LOG(LogNexusPicker, Verbose, TEXT("Unable to project a location to the NavMesh! Was the location(%s) outside of the NavMesh volume, or inside of a NavModifier. Adjust your NavQueryExtent to account for this increased distance, or ensure your dimensions are within the NavMesh volume."), *Location.ToCompactString());
						++NavMeshProjectionFailures;
					}
				}
				else
				{
					UE_LOG(LogNexusPicker, Error, TEXT("Unable to project a location to the NavMesh as NavigationSystemV1 was null."));
				}
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int32 i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(GeneratePoint());
			}
		}
	}
};
