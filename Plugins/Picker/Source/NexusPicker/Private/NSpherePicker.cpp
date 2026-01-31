// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSpherePicker.h"
#include "NavigationSystem.h"
#include "NPickerMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_SPHERE_PREFIX \
	const int OutLocationsStartIndex = OutLocations.Num(); \
	const bool bSimpleMode = !(Params.MinimumRadius > 0.f); \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count);
#define N_PICKER_SPHERE_LOCATION_SIMPLE(VectorValue) \
	Params.Origin + (VectorValue() * Params.MaximumRadius)
#define N_PICKER_SPHERE_LOCATION(VectorValue, FloatRange) \
	Params.Origin + (VectorValue() * FloatRange(Params.MinimumRadius, Params.MaximumRadius))

#if ENABLE_VISUAL_LOG
#define N_PICKER_SPHERE_VLOG(HasMinimumDimensions) \
	if(Params.CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		if(HasMinimumDimensions) \
		{ \
			UE_VLOG_WIRESPHERE(Params.CachedWorld, LogNexusPicker, Verbose, Params.Origin, Params.MinimumRadius, NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		} \
		UE_VLOG_WIRESPHERE(Params.CachedWorld, LogNexusPicker, Verbose, Params.Origin, Params.MaximumRadius, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		for (int i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(Params.CachedWorld , LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else
#define N_PICKER_SPHERE_VLOG(HasMinimumDimensions)
#endif

// #SONARQUBE-DISABLE-CPP_S107 Lot of boilerplate code here
// Excluded from code duplication

#define RANDOM_VECTOR FNRandom::Deterministic.VRand
#define RANDOM_FLOAT_RANGE FNRandom::Deterministic.FloatRange
void FNSpherePicker::Next(TArray<FVector>& OutLocations, const FNSpherePickerParams& Params)
{
	N_PICKER_SPHERE_PREFIX
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_SPHERE_LOCATION_SIMPLE(RANDOM_VECTOR);
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_SPHERE_LOCATION_SIMPLE(RANDOM_VECTOR);
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_SPHERE_LOCATION_SIMPLE(RANDOM_VECTOR));
			}
		}
	}
	else
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR, RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR, RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR, RANDOM_FLOAT_RANGE));
			}
		}
	}
	N_PICKER_SPHERE_VLOG(!bSimpleMode)
}
#undef RANDOM_VECTOR
#undef RANDOM_FLOAT_RANGE

#define RANDOM_VECTOR FNRandom::NonDeterministic.VRand
#define RANDOM_FLOAT_RANGE FNRandom::NonDeterministic.FRandRange
void FNSpherePicker::Random(TArray<FVector>& OutLocations, const FNSpherePickerParams& Params)
{
	N_PICKER_SPHERE_PREFIX
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_SPHERE_LOCATION_SIMPLE(RANDOM_VECTOR);
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_SPHERE_LOCATION_SIMPLE(RANDOM_VECTOR);
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_SPHERE_LOCATION_SIMPLE(RANDOM_VECTOR));
			}
		}
	}
	else
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR, RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR, RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR, RANDOM_FLOAT_RANGE));
			}
		}
	}
	N_PICKER_SPHERE_VLOG(!bSimpleMode)	
}
#undef RANDOM_VECTOR
#undef RANDOM_FLOAT_RANGE

#define RANDOM_VECTOR RandomStream.VRand
#define RANDOM_FLOAT_RANGE RandomStream.FRandRange
void FNSpherePicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNSpherePickerParams& Params)
{
	const FRandomStream RandomStream(Seed);
	N_PICKER_SPHERE_PREFIX
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_SPHERE_LOCATION_SIMPLE(RANDOM_VECTOR);
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_SPHERE_LOCATION_SIMPLE(RANDOM_VECTOR);
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_SPHERE_LOCATION_SIMPLE(RANDOM_VECTOR));
			}
		}
	}
	else
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR, RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR, RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR, RANDOM_FLOAT_RANGE));
			}
		}
	}
	N_PICKER_SPHERE_VLOG(!bSimpleMode)
	Seed = RandomStream.GetCurrentSeed();
}
#undef RANDOM_VECTOR
#undef RANDOM_FLOAT_RANGE

// #SONARQUBE-ENABLE