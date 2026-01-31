// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NBoxPicker.h"

#include "NavigationSystem.h"
#include "NPickerMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_BOX_PREFIX \
	const int OutLocationsStartIndex = OutLocations.Num(); \
	const bool bSimpleMode = Params.MinimumDimensions.IsValid == 0; \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count);
#define N_PICKER_BOX_EXTENTS \
	const FVector MinimumExtent = 0.5f * (Params.MinimumDimensions.Max - Params.MinimumDimensions.Min); \
	const FVector MaximumExtent = 0.5f * (Params.MaximumDimensions.Max - Params.MaximumDimensions.Min);
#define N_PICKER_BOX_LOCATION_SIMPLE(FloatValue) \
	Params.Origin + FVector( \
		FloatValue(Params.MaximumDimensions.Min.X, Params.MaximumDimensions.Max.X), \
		FloatValue(Params.MaximumDimensions.Min.Y, Params.MaximumDimensions.Max.Y), \
		FloatValue(Params.MaximumDimensions.Min.Z, Params.MaximumDimensions.Max.Z))
#define N_PICKER_BOX_LOCATION(FloatValue, BooleanValue) \
	Params.Origin + FVector( \
		BooleanValue ? FloatValue(-MinimumExtent.X, -MaximumExtent.X) : FloatValue(MinimumExtent.X, MaximumExtent.X), \
		BooleanValue ? FloatValue(-MinimumExtent.Y, -MaximumExtent.Y) : FloatValue(MinimumExtent.Y, MaximumExtent.Y), \
		BooleanValue ? FloatValue(-MinimumExtent.Z, -MaximumExtent.Z) : FloatValue(MinimumExtent.Z, MaximumExtent.Z))

#if ENABLE_VISUAL_LOG
#define N_PICKER_BOX_VLOG(HasMinimumDimensions) \
	if(Params.CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		if(HasMinimumDimensions) \
		{ \
			UE_VLOG_WIREBOX(Params.CachedWorld , LogNexusPicker, Verbose, Params.MinimumDimensions.MoveTo(Params.Origin), NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		} \
		if(Params.MaximumDimensions.IsValid != 0) \
		{ \
			UE_VLOG_WIREBOX(Params.CachedWorld , LogNexusPicker, Verbose, Params.MaximumDimensions.MoveTo(Params.Origin), NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		} \
		for (int i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(Params.CachedWorld , LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else
#define N_PICKER_BOX_VLOG(HasMinimumDimensions)
#endif

// #SONARQUBE-DISABLE-CPP_S107 Lot of boilerplate code here

#define RANDOM_FLOAT_RANGE FNRandom::Deterministic.FloatRange
#define RANDOM_BOOL FNRandom::Deterministic.Bool()
void FNBoxPicker::Next(TArray<FVector>& OutLocations, const FNBoxPickerParams& Params)
{
	N_PICKER_BOX_PREFIX
	
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE));
			}
		}
	}
	else
	{
		N_PICKER_BOX_EXTENTS
		
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL));
			}
		}
	}
	
	N_PICKER_BOX_VLOG(!bSimpleMode)
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_BOOL

#define RANDOM_FLOAT_RANGE FNRandom::NonDeterministic.FRandRange
#define RANDOM_BOOL FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f
void FNBoxPicker::Random(TArray<FVector>& OutLocations, const FNBoxPickerParams& Params)
{
	N_PICKER_BOX_PREFIX
	
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE));
			}
		}
	}
	else
	{
		N_PICKER_BOX_EXTENTS
		
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL));
			}
		}
	}
	
	N_PICKER_BOX_VLOG(!bSimpleMode)
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_BOOL

#define RANDOM_FLOAT_RANGE RandomStream.FRandRange
#define RANDOM_BOOL RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f
void FNBoxPicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNBoxPickerParams& Params)
{
	const FRandomStream RandomStream(Seed);
	N_PICKER_BOX_PREFIX
	if (bSimpleMode)
	{
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE));
			}
		}
	}
	else
	{
		N_PICKER_BOX_EXTENTS
		
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				FVector Location = N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				OutLocations.Add(N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL));
			}
		}
	}
	
	N_PICKER_BOX_VLOG(!bSimpleMode)
	Seed = RandomStream.GetCurrentSeed();
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_BOOL

// #SONARQUBE-ENABLE