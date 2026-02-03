// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NRadialPicker.h"

#include "NavigationSystem.h"
#include "NPickerMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_RADIAL_LOCATION \
	FVector Location = Params.Origin + Params.Rotation.RotateVector(FVector(FMath::Cos(RandomAngle), FMath::Sin(RandomAngle), 0) * RandomDistance);

#define RANDOM_FLOAT_RANGE FNRandom::Deterministic.FloatRange
void FNRadialPicker::Next(TArray<FVector>& OutLocations, const FNRadialPickerParams& Params)
{
	if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			const float RandomAngle = FMath::DegreesToRadians(RANDOM_FLOAT_RANGE(Params.MinimumAngle, Params.MaximumAngle));
			const float RandomDistance = RANDOM_FLOAT_RANGE(Params.MinimumDistance, Params.MaximumDistance);
			N_PICKER_RADIAL_LOCATION
			N_IMPLEMENT_PICKER_PROJECTION_TRACE
			OutLocations.Add(Location);
		}
	}
	else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			const float RandomAngle = FMath::DegreesToRadians(RANDOM_FLOAT_RANGE(Params.MinimumAngle, Params.MaximumAngle));
			const float RandomDistance = RANDOM_FLOAT_RANGE(Params.MinimumDistance, Params.MaximumDistance);
			N_PICKER_RADIAL_LOCATION
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
			OutLocations.Add(Location);
		}
	}
	else
	{
		for (int i = 0; i < Params.Count; i++)
		{
			const float RandomAngle = FMath::DegreesToRadians(RANDOM_FLOAT_RANGE(Params.MinimumAngle, Params.MaximumAngle));
			const float RandomDistance = RANDOM_FLOAT_RANGE(Params.MinimumDistance, Params.MaximumDistance);
			N_PICKER_RADIAL_LOCATION
			OutLocations.Add(Location);
		}
	}
}
#undef RANDOM_FLOAT_RANGE

#define RANDOM_FLOAT_RANGE FNRandom::NonDeterministic.FRandRange
void FNRadialPicker::Random(TArray<FVector>& OutLocations, const FNRadialPickerParams& Params)
{
	if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			const float RandomAngle = FMath::DegreesToRadians(RANDOM_FLOAT_RANGE(Params.MinimumAngle, Params.MaximumAngle));
			const float RandomDistance = RANDOM_FLOAT_RANGE(Params.MinimumDistance, Params.MaximumDistance);
			N_PICKER_RADIAL_LOCATION
			N_IMPLEMENT_PICKER_PROJECTION_TRACE
			OutLocations.Add(Location);
		}
	}
	else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			const float RandomAngle = FMath::DegreesToRadians(RANDOM_FLOAT_RANGE(Params.MinimumAngle, Params.MaximumAngle));
			const float RandomDistance = RANDOM_FLOAT_RANGE(Params.MinimumDistance, Params.MaximumDistance);
			N_PICKER_RADIAL_LOCATION
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
			OutLocations.Add(Location);
		}
	}
	else
	{
		for (int i = 0; i < Params.Count; i++)
		{
			const float RandomAngle = FMath::DegreesToRadians(RANDOM_FLOAT_RANGE(Params.MinimumAngle, Params.MaximumAngle));
			const float RandomDistance = RANDOM_FLOAT_RANGE(Params.MinimumDistance, Params.MaximumDistance);
			N_PICKER_RADIAL_LOCATION
			OutLocations.Add(Location);
		}
	}
}
#undef RANDOM_FLOAT_RANGE

#define RANDOM_FLOAT_RANGE RandomStream.FRandRange
void FNRadialPicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNRadialPickerParams& Params)
{
	const FRandomStream RandomStream(Seed);
	if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			const float RandomAngle = FMath::DegreesToRadians(RANDOM_FLOAT_RANGE(Params.MinimumAngle, Params.MaximumAngle));
			const float RandomDistance = RANDOM_FLOAT_RANGE(Params.MinimumDistance, Params.MaximumDistance);
			N_PICKER_RADIAL_LOCATION
			N_IMPLEMENT_PICKER_PROJECTION_TRACE
			OutLocations.Add(Location);
		}
	}
	else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			const float RandomAngle = FMath::DegreesToRadians(RANDOM_FLOAT_RANGE(Params.MinimumAngle, Params.MaximumAngle));
			const float RandomDistance = RANDOM_FLOAT_RANGE(Params.MinimumDistance, Params.MaximumDistance);
			N_PICKER_RADIAL_LOCATION
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
			OutLocations.Add(Location);
		}
	}
	else
	{
		for (int i = 0; i < Params.Count; i++)
		{
			const float RandomAngle = FMath::DegreesToRadians(RANDOM_FLOAT_RANGE(Params.MinimumAngle, Params.MaximumAngle));
			const float RandomDistance = RANDOM_FLOAT_RANGE(Params.MinimumDistance, Params.MaximumDistance);
			N_PICKER_RADIAL_LOCATION
			OutLocations.Add(Location);
		}
	}
	Seed = RandomStream.GetCurrentSeed();
}
#undef RANDOM_FLOAT_RANGE
