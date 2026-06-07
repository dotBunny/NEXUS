// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCirclePicker.h"
#include "NavigationSystem.h"
#include "NPickerMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_CIRCLE_PREFIX \
	const int32 OutLocationsStartIndex = OutLocations.Num(); \
	const bool bSimpleMode = Params.Rotation.IsZero(); \
	const float MinRadiusSq = Params.MinimumRadius * Params.MinimumRadius; \
	const float MaxRadiusSq = Params.MaximumRadius * Params.MaximumRadius; \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count); \
	if (Params.MinimumRadius < 0.f || Params.MaximumRadius < Params.MinimumRadius) \
	{ \
		UE_LOG(LogNexusPicker, Warning, TEXT("FNCirclePickerParams has invalid radius range (Min=%.2f, Max=%.2f); points may not be where you expect."), Params.MinimumRadius, Params.MaximumRadius); \
	}
#define N_PICKER_CIRCLE_THETA(FloatSingle) \
	const float PointTheta = Random.FloatSingle() * 2.0f * PI;
// Inverse-CDF transform: uniform u in [0,1] maps to area-weighted radius across the annulus.
#define N_PICKER_CIRCLE_RADIUS(FloatSingle) \
	const float PointRadius = FMath::Sqrt(Random.FloatSingle() * (MaxRadiusSq - MinRadiusSq) + MinRadiusSq);
#define N_PICKER_CIRCLE_LOCATION_SIMPLE \
	Params.Origin + FVector((PointRadius * FMath::Cos(PointTheta)),(PointRadius * FMath::Sin(PointTheta)), 0.f)
#define N_PICKER_CIRCLE_LOCATION \
	Params.Origin + Params.Rotation.RotateVector(FVector((PointRadius * FMath::Cos(PointTheta)),(PointRadius * FMath::Sin(PointTheta)), 0.f))

#if ENABLE_VISUAL_LOG
#define N_PICKER_CIRCLE_VLOG(HasMinimumDimensions) \
	if(Params.CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		if(HasMinimumDimensions) \
		{ \
			UE_VLOG_WIRECIRCLE(Params.CachedWorld, LogNexusPicker, Verbose, Params.Origin, Params.Rotation.RotateVector(FVector::UpVector), Params.MinimumRadius, NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		} \
		UE_VLOG_WIRECIRCLE(Params.CachedWorld, LogNexusPicker, Verbose, Params.Origin, Params.Rotation.RotateVector(FVector::UpVector), Params.MaximumRadius, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		for (int32 i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(Params.CachedWorld, LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else // !ENABLE_VISUAL_LOG
#define N_PICKER_CIRCLE_VLOG(HasMinimumDimensions)
#endif // ENABLE_VISUAL_LOG

// #SONARQUBE-DISABLE-CPP_S107 Lot of boilerplate code here
// Excluded from code duplication

#define RANDOM_FLOAT Float
void FNCirclePicker::Next(TArray<FVector>& OutLocations, const FNCirclePickerParams& Params)
{
	N_PICKER_RANDOM_DETERMINISTIC
	N_PICKER_CIRCLE_PREFIX
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION_SIMPLE);
			}
		}
	}
	else
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION);
			}
		}
	}
	
	N_PICKER_CIRCLE_VLOG(Params.MinimumRadius > 0.f)
}
#undef RANDOM_FLOAT

#define RANDOM_FLOAT FRand
void FNCirclePicker::Random(TArray<FVector>& OutLocations, const FNCirclePickerParams& Params)
{
	N_PICKER_RANDOM_NONDETERMINISTIC
	N_PICKER_CIRCLE_PREFIX
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION_SIMPLE);
			}
		}
	}
	else
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION);
			}
		}
	}
	
	N_PICKER_CIRCLE_VLOG(Params.MinimumRadius > 0.f)
}
#undef RANDOM_FLOAT

#define RANDOM_FLOAT FRand
void FNCirclePicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNCirclePickerParams& Params)
{
	const FRandomStream Random(Seed);
	N_PICKER_CIRCLE_PREFIX
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION_SIMPLE);
			}
		}
	}
	else
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION);
			}
		}
	}
	
	N_PICKER_CIRCLE_VLOG(Params.MinimumRadius > 0.f)
	Seed = Random.GetCurrentSeed();
}
#undef RANDOM_FLOAT

#define RANDOM_FLOAT Float
void FNCirclePicker::Twisted(TArray<FVector>& OutLocations, FNMersenneTwister& Random, const FNCirclePickerParams& Params)
{
	N_PICKER_CIRCLE_PREFIX
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION_SIMPLE);
			}
		}
	}
	else
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION);
			}
		}
	}
	
	N_PICKER_CIRCLE_VLOG(Params.MinimumRadius > 0.f)
}
#undef RANDOM_FLOAT

// #SONARQUBE-ENABLE
