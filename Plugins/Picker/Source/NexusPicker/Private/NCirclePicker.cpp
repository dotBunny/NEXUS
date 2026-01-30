// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCirclePicker.h"
#include "NavigationSystem.h"
#include "NPickerMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_CIRCLE_PREFIX \
	const int OutLocationsStartIndex = OutLocations.Num(); \
	const bool bSimpleMode = Params.Rotation.IsZero(); \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count);
#define N_PICKER_CIRCLE_THETA(FloatSingle) \
	const float PointTheta = FloatSingle() * 2.0f * PI;
#define N_PICKER_CIRCLE_RADIUS(FloatValue) \
	const float PointRadius = FloatValue(Params.MinimumRadius, Params.MaximumRadius);
#define N_PICKER_CIRCLE_LOCATION_SIMPLE \
	Params.Origin + FVector((PointRadius * FMath::Cos(PointTheta)),(PointRadius * FMath::Sin(PointTheta)), 0.f)
#define N_PICKER_CIRCLE_LOCATION \
	Params.Origin + Params.Rotation.RotateVector(FVector((PointRadius * FMath::Cos(PointTheta)),(PointRadius * FMath::Sin(PointTheta)), 0.f))

#if ENABLE_VISUAL_LOG
#define N_PICKER_CIRCLE_VLOG \
	if(Params.CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		UE_VLOG_WIRECIRCLE(Params.CachedWorld, LogNexusPicker, Verbose, Params.Origin, Params.Rotation.RotateVector(FVector::UpVector), Params.MinimumRadius, NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		UE_VLOG_WIRECIRCLE(Params.CachedWorld, LogNexusPicker, Verbose, Params.Origin, Params.Rotation.RotateVector(FVector::UpVector), Params.MaximumRadius, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		for (int i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(Params.CachedWorld, LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else
#define N_PICKER_CIRCLE_VLOG
#endif

// #SONARQUBE-DISABLE-CPP_S107 Trying to keep all logic in one method

void FNCirclePicker::Next(TArray<FVector>& OutLocations, const FNCirclePickerParams& Params)
{
	N_PICKER_CIRCLE_PREFIX
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(FNRandom::Deterministic.Float)
				N_PICKER_CIRCLE_RADIUS(FNRandom::Deterministic.FloatRange)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(FNRandom::Deterministic.Float)
				N_PICKER_CIRCLE_RADIUS(FNRandom::Deterministic.FloatRange)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(FNRandom::Deterministic.Float)
				N_PICKER_CIRCLE_RADIUS(FNRandom::Deterministic.FloatRange)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION_SIMPLE);
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
				N_PICKER_CIRCLE_THETA(FNRandom::Deterministic.Float)
				N_PICKER_CIRCLE_RADIUS(FNRandom::Deterministic.FloatRange)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(FNRandom::Deterministic.Float)
				N_PICKER_CIRCLE_RADIUS(FNRandom::Deterministic.FloatRange)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(FNRandom::Deterministic.Float)
				N_PICKER_CIRCLE_RADIUS(FNRandom::Deterministic.FloatRange)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION);
			}
		}
	}
	
	N_PICKER_CIRCLE_VLOG
}

void FNCirclePicker::Random(TArray<FVector>& OutLocations, const FNCirclePickerParams& Params)
{
	N_PICKER_CIRCLE_PREFIX
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(FNRandom::NonDeterministic.FRand)
				N_PICKER_CIRCLE_RADIUS(FNRandom::NonDeterministic.FRandRange)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(FNRandom::NonDeterministic.FRand)
				N_PICKER_CIRCLE_RADIUS(FNRandom::NonDeterministic.FRandRange)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(FNRandom::NonDeterministic.FRand)
				N_PICKER_CIRCLE_RADIUS(FNRandom::NonDeterministic.FRandRange)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION_SIMPLE);
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
				N_PICKER_CIRCLE_THETA(FNRandom::NonDeterministic.FRand)
				N_PICKER_CIRCLE_RADIUS(FNRandom::NonDeterministic.FRandRange)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(FNRandom::NonDeterministic.FRand)
				N_PICKER_CIRCLE_RADIUS(FNRandom::NonDeterministic.FRandRange)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(FNRandom::NonDeterministic.FRand)
				N_PICKER_CIRCLE_RADIUS(FNRandom::NonDeterministic.FRandRange)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION);
			}
		}
	}
	
	N_PICKER_CIRCLE_VLOG
}

void FNCirclePicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNCirclePickerParams& Params)
{
	N_PICKER_CIRCLE_PREFIX
	
	const FRandomStream RandomStream(Seed);
	
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RandomStream.FRand)
				N_PICKER_CIRCLE_RADIUS(RandomStream.FRandRange)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RandomStream.FRand)
				N_PICKER_CIRCLE_RADIUS(RandomStream.FRandRange)
				FVector Location = N_PICKER_CIRCLE_LOCATION_SIMPLE;
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RandomStream.FRand)
				N_PICKER_CIRCLE_RADIUS(RandomStream.FRandRange)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION_SIMPLE);
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
				N_PICKER_CIRCLE_THETA(RandomStream.FRand)
				N_PICKER_CIRCLE_RADIUS(RandomStream.FRandRange)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RandomStream.FRand)
				N_PICKER_CIRCLE_RADIUS(RandomStream.FRandRange)
				FVector Location = N_PICKER_CIRCLE_LOCATION;
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_CIRCLE_THETA(RandomStream.FRand)
				N_PICKER_CIRCLE_RADIUS(RandomStream.FRandRange)
				OutLocations.Add(N_PICKER_CIRCLE_LOCATION);
			}
		}
	}
	
	N_PICKER_CIRCLE_VLOG
	Seed = RandomStream.GetCurrentSeed();
}

// #SONARQUBE-ENABLE
