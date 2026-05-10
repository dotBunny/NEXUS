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
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count);
#define N_PICKER_CIRCLE_THETA(FloatSingle) \
	const float PointTheta = Random.FloatSingle() * 2.0f * PI;
#define N_PICKER_CIRCLE_RADIUS(FloatValue) \
	const float PointRadius = Random.FloatValue(Params.MinimumRadius, Params.MaximumRadius);
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

#define RANDOM_FLOAT_RANGE FloatRange
#define RANDOM_FLOAT Float
void FNCirclePicker::Next(TArray<FVector>& OutLocations, const FNCirclePickerParams& Params)
{
	N_IMPLEMENT_PICKER_RANDOM_DETERMINISTIC
	N_PICKER_CIRCLE_PREFIX
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION_SIMPLE);
			}
		}
	}
	else
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION);
			}
		}
	}
	
	N_PICKER_CIRCLE_VLOG(!bSimpleMode)
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_FLOAT

#define RANDOM_FLOAT_RANGE FRandRange
#define RANDOM_FLOAT FRand
void FNCirclePicker::Random(TArray<FVector>& OutLocations, const FNCirclePickerParams& Params)
{
	N_IMPLEMENT_PICKER_RANDOM_NONDETERMINISTIC
	N_PICKER_CIRCLE_PREFIX
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION_SIMPLE);
			}
		}
	}
	else
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION);
			}
		}
	}
	
	N_PICKER_CIRCLE_VLOG(!bSimpleMode)
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_FLOAT

#define RANDOM_FLOAT_RANGE FRandRange
#define RANDOM_FLOAT FRand
void FNCirclePicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNCirclePickerParams& Params)
{
	const FRandomStream Random(Seed);
	N_PICKER_CIRCLE_PREFIX
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION_SIMPLE);
			}
		}
	}
	else
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION);
			}
		}
	}
	
	N_PICKER_CIRCLE_VLOG(!bSimpleMode)
	Seed = Random.GetCurrentSeed();
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_FLOAT

#define RANDOM_FLOAT_RANGE FloatRange
#define RANDOM_FLOAT Float
void FNCirclePicker::Twisted(TArray<FVector>& OutLocations, FNMersenneTwister& Random, const FNCirclePickerParams& Params)
{
	N_PICKER_CIRCLE_PREFIX
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION_SIMPLE);
			}
		}
	}
	else
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int32 i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RANDOM_FLOAT)
				N_PICKER_CIRCLE_RADIUS(RANDOM_FLOAT_RANGE)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION);
			}
		}
	}
	
	N_PICKER_CIRCLE_VLOG(!bSimpleMode)
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_FLOAT

// #SONARQUBE-ENABLE
