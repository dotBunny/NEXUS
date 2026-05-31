// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSpherePicker.h"
#include "NavigationSystem.h"
#include "NPickerMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_SPHERE_PREFIX \
	const int32 OutLocationsStartIndex = OutLocations.Num(); \
	const float MinRadiusCubed = Params.MinimumRadius * Params.MinimumRadius * Params.MinimumRadius; \
	const float MaxRadiusCubed = Params.MaximumRadius * Params.MaximumRadius * Params.MaximumRadius; \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count);
// Inverse-CDF transform: uniform u in [0,1] maps to volume-weighted radius across the shell.
#define N_PICKER_SPHERE_RADIUS(FloatSingle) \
	const float PointRadius = FMath::Pow(Random.FloatSingle() * (MaxRadiusCubed - MinRadiusCubed) + MinRadiusCubed, 1.0f / 3.0f);
#define N_PICKER_SPHERE_LOCATION(VectorValue) \
	Params.Origin + (Random.VectorValue() * PointRadius)

#if ENABLE_VISUAL_LOG
#define N_PICKER_SPHERE_VLOG \
	if(Params.CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		UE_VLOG_WIRESPHERE(Params.CachedWorld, LogNexusPicker, Verbose, Params.Origin, Params.MinimumRadius, NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		UE_VLOG_WIRESPHERE(Params.CachedWorld, LogNexusPicker, Verbose, Params.Origin, Params.MaximumRadius, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		for (int32 i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(Params.CachedWorld , LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else // !ENABLE_VISUAL_LOG
#define N_PICKER_SPHERE_VLOG
#endif // ENABLE_VISUAL_LOG

// #SONARQUBE-DISABLE-CPP_S107 Lot of boilerplate code here
// Excluded from code duplication

#define RANDOM_VECTOR VRand
#define RANDOM_FLOAT Float
void FNSpherePicker::Next(TArray<FVector>& OutLocations, const FNSpherePickerParams& Params)
{
	N_IMPLEMENT_PICKER_RANDOM_DETERMINISTIC
	N_PICKER_SPHERE_PREFIX
	if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
		for (int32 i = 0; i < Params.Count; i++)
		{
			N_PICKER_SPHERE_RADIUS(RANDOM_FLOAT)
			FVector Location = N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR);
			N_IMPLEMENT_PICKER_PROJECTION_TRACE
			OutLocations.Add(Location);
		}
	}
	else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
		for (int32 i = 0; i < Params.Count; i++)
		{
			N_PICKER_SPHERE_RADIUS(RANDOM_FLOAT)
			FVector Location = N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR);
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
			OutLocations.Add(Location);
		}
	}
	else
	{
		for (int32 i = 0; i < Params.Count; i++)
		{
			N_PICKER_SPHERE_RADIUS(RANDOM_FLOAT)
			OutLocations.Add(N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR));
		}
	}
	N_PICKER_SPHERE_VLOG
}
#undef RANDOM_VECTOR
#undef RANDOM_FLOAT

#define RANDOM_VECTOR VRand
#define RANDOM_FLOAT FRand
void FNSpherePicker::Random(TArray<FVector>& OutLocations, const FNSpherePickerParams& Params)
{
	N_IMPLEMENT_PICKER_RANDOM_NONDETERMINISTIC
	N_PICKER_SPHERE_PREFIX
	if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
		for (int32 i = 0; i < Params.Count; i++)
		{
			N_PICKER_SPHERE_RADIUS(RANDOM_FLOAT)
			FVector Location = N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR);
			N_IMPLEMENT_PICKER_PROJECTION_TRACE
			OutLocations.Add(Location);
		}
	}
	else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
		for (int32 i = 0; i < Params.Count; i++)
		{
			N_PICKER_SPHERE_RADIUS(RANDOM_FLOAT)
			FVector Location = N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR);
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
			OutLocations.Add(Location);
		}
	}
	else
	{
		for (int32 i = 0; i < Params.Count; i++)
		{
			N_PICKER_SPHERE_RADIUS(RANDOM_FLOAT)
			OutLocations.Add(N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR));
		}
	}
	N_PICKER_SPHERE_VLOG
}
#undef RANDOM_VECTOR
#undef RANDOM_FLOAT

#define RANDOM_VECTOR VRand
#define RANDOM_FLOAT FRand
void FNSpherePicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNSpherePickerParams& Params)
{
	const FRandomStream Random(Seed);
	N_PICKER_SPHERE_PREFIX
	if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
		for (int32 i = 0; i < Params.Count; i++)
		{
			N_PICKER_SPHERE_RADIUS(RANDOM_FLOAT)
			FVector Location = N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR);
			N_IMPLEMENT_PICKER_PROJECTION_TRACE
			OutLocations.Add(Location);
		}
	}
	else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
		for (int32 i = 0; i < Params.Count; i++)
		{
			N_PICKER_SPHERE_RADIUS(RANDOM_FLOAT)
			FVector Location = N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR);
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
			OutLocations.Add(Location);
		}
	}
	else
	{
		for (int32 i = 0; i < Params.Count; i++)
		{
			N_PICKER_SPHERE_RADIUS(RANDOM_FLOAT)
			OutLocations.Add(N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR));
		}
	}

	N_PICKER_SPHERE_VLOG
	Seed = Random.GetCurrentSeed();
}
#undef RANDOM_VECTOR
#undef RANDOM_FLOAT

#define RANDOM_VECTOR VRand
#define RANDOM_FLOAT Float
void FNSpherePicker::Twisted(TArray<FVector>& OutLocations, FNMersenneTwister& Random, const FNSpherePickerParams& Params)
{
	N_PICKER_SPHERE_PREFIX
	if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
		for (int32 i = 0; i < Params.Count; i++)
		{
			N_PICKER_SPHERE_RADIUS(RANDOM_FLOAT)
			FVector Location = N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR);
			N_IMPLEMENT_PICKER_PROJECTION_TRACE
			OutLocations.Add(Location);
		}
	}
	else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
		for (int32 i = 0; i < Params.Count; i++)
		{
			N_PICKER_SPHERE_RADIUS(RANDOM_FLOAT)
			FVector Location = N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR);
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
			OutLocations.Add(Location);
		}
	}
	else
	{
		for (int32 i = 0; i < Params.Count; i++)
		{
			N_PICKER_SPHERE_RADIUS(RANDOM_FLOAT)
			OutLocations.Add(N_PICKER_SPHERE_LOCATION(RANDOM_VECTOR));
		}
	}
	N_PICKER_SPHERE_VLOG
}
#undef RANDOM_VECTOR
#undef RANDOM_FLOAT

// #SONARQUBE-ENABLE