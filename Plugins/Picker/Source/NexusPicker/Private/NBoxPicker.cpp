// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NBoxPicker.h"

#include "NavigationSystem.h"
#include "NPickerMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_BOX_PREFIX \
	const int OutLocationsStartIndex = OutLocations.Num(); \
	const bool bSimpleMode = Params.MinimumBox.IsValid == 0; \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count);
#define N_PICKER_BOX_VALID_BOXES_CHOICE(RandomIndex) \
	FBox ChosenBox = ValidBoxes[RandomIndex(0,ValidBoxes.Num()-1)];
#define N_PICKER_BOX_LOCATION_SIMPLE(FloatValue) \
	Params.Origin + FVector( \
		FloatValue(Params.MaximumBox.Min.X, Params.MaximumBox.Max.X), \
		FloatValue(Params.MaximumBox.Min.Y, Params.MaximumBox.Max.Y), \
		FloatValue(Params.MaximumBox.Min.Z, Params.MaximumBox.Max.Z))
#define N_PICKER_BOX_LOCATION(FloatValue) \
	Params.Origin + FVector( \
		FloatValue(ChosenBox.Min.X, ChosenBox.Max.X), \
		FloatValue(ChosenBox.Min.Y, ChosenBox.Max.Y), \
		FloatValue(ChosenBox.Min.Z, ChosenBox.Max.Z))

#if ENABLE_VISUAL_LOG
#define N_PICKER_BOX_VALID_BOXES \
	TArray<FBox> ValidBoxes = Params.GetValidBoxes(); \
	if(Params.CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		for(int v = 0; v < ValidBoxes.Num(); v++) \
		{ \
			UE_VLOG_WIREBOX(Params.CachedWorld , LogNexusPicker, Verbose, ValidBoxes[v].ShiftBy(Params.Origin), NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		} \
	}
#define N_PICKER_BOX_VLOG(HasMinimumBox) \
	if(Params.CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		if(HasMinimumBox) \
		{ \
			UE_VLOG_WIREBOX(Params.CachedWorld , LogNexusPicker, Verbose, Params.MinimumBox.ShiftBy(Params.Origin), NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		} \
		if(Params.MaximumBox.IsValid != 0) \
		{ \
			UE_VLOG_WIREBOX(Params.CachedWorld , LogNexusPicker, Verbose, Params.MaximumBox.ShiftBy(Params.Origin), NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		} \
		for (int i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(Params.CachedWorld , LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else // ENABLE_VISUAL_LOG
#define N_PICKER_BOX_VLOG(HasMinimumBox)
#define N_PICKER_BOX_VALID_BOXES \
	TArray<FBox> ValidBoxes = Params.GetValidBoxes();
#endif // ENABLE_VISUAL_LOG


// #SONARQUBE-DISABLE-CPP_S107 Lot of boilerplate code here
// Excluded from code duplication

#define RANDOM_INDEX FNRandom::Deterministic.IntegerRange
#define RANDOM_FLOAT_RANGE FNRandom::Deterministic.FloatRange
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
		N_PICKER_BOX_VALID_BOXES
		
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_BOX_VALID_BOXES_CHOICE(RANDOM_INDEX)
				FVector Location = N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_BOX_VALID_BOXES_CHOICE(RANDOM_INDEX)
				FVector Location = N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_BOX_VALID_BOXES_CHOICE(RANDOM_INDEX)
				OutLocations.Add(N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE));
			}
		}
	}
	
	N_PICKER_BOX_VLOG(!bSimpleMode)
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_INDEX

#define RANDOM_FLOAT_RANGE FNRandom::NonDeterministic.FRandRange
#define RANDOM_INDEX FNRandom::NonDeterministic.RandRange
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
		N_PICKER_BOX_VALID_BOXES
		
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_BOX_VALID_BOXES_CHOICE(RANDOM_INDEX)
				FVector Location = N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_BOX_VALID_BOXES_CHOICE(RANDOM_INDEX)
				FVector Location = N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_BOX_VALID_BOXES_CHOICE(RANDOM_INDEX)
				OutLocations.Add(N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE));
			}
		}
	}
	
	N_PICKER_BOX_VLOG(!bSimpleMode)
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_INDEX

#define RANDOM_FLOAT_RANGE RandomStream.FRandRange
#define RANDOM_INDEX RandomStream.RandRange
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
		N_PICKER_BOX_VALID_BOXES
		
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_BOX_VALID_BOXES_CHOICE(RANDOM_INDEX)
				FVector Location = N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_TRACE
				OutLocations.Add(Location);
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_BOX_VALID_BOXES_CHOICE(RANDOM_INDEX)
				FVector Location = N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE);
				N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
				OutLocations.Add(Location);
			}
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				N_PICKER_BOX_VALID_BOXES_CHOICE(RANDOM_INDEX)
				OutLocations.Add(N_PICKER_BOX_LOCATION(RANDOM_FLOAT_RANGE));
			}
		}
	}
	
	N_PICKER_BOX_VLOG(!bSimpleMode)
	Seed = RandomStream.GetCurrentSeed();
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_INDEX

// #SONARQUBE-ENABLE