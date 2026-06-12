// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NOrientedBoxPicker.h"
#include "Algo/BinarySearch.h"
#include "NavigationSystem.h"
#include "NPickerMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_ORIENTED_BOX_PREFIX \
	N_PICKER_PARAMS_WORLD_SAFETY \
	const int32 OutLocationsStartIndex = OutLocations.Num(); \
	const bool bSimpleMode = Params.MinimumDimensions.IsZero(); \
	const bool bHasRotation = !Params.Rotation.IsZero(); \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count);
#define N_PICKER_ORIENTED_BOX_EXTENTS_SIMPLE \
	const FVector MaximumExtent = 0.5f * Params.MaximumDimensions;
#define N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE(FloatValue) \
	Params.Origin + FVector( \
		Random.FloatValue(-MaximumExtent.X, MaximumExtent.X), \
		Random.FloatValue(-MaximumExtent.Y, MaximumExtent.Y), \
		Random.FloatValue(-MaximumExtent.Z, MaximumExtent.Z))
#define N_PICKER_ORIENTED_BOX_CUMULATIVE \
	TArray<double> CumulativeVolumes; \
	CumulativeVolumes.Reserve(ValidBoxes.Num()); \
	double TotalVolume = 0.0; \
	for (const FBox& CumulativeBox : ValidBoxes) \
	{ \
		TotalVolume += CumulativeBox.GetVolume(); \
		CumulativeVolumes.Add(TotalVolume); \
	}
#define N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(FloatValue) \
	const double VolumePick = Random.FloatValue(0.f, 1.f) * TotalVolume; \
	const int32 ChosenIndex = FMath::Min(Algo::LowerBound(CumulativeVolumes, VolumePick), ValidBoxes.Num() - 1); \
	const FBox ChosenBox = ValidBoxes[ChosenIndex];
#define N_PICKER_ORIENTED_BOX_LOCATION(FloatValue) \
	Params.Origin + FVector( \
		Random.FloatValue(ChosenBox.Min.X, ChosenBox.Max.X), \
		Random.FloatValue(ChosenBox.Min.Y, ChosenBox.Max.Y), \
		Random.FloatValue(ChosenBox.Min.Z, ChosenBox.Max.Z))
#define N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE_ROTATION(FloatValue) \
	Params.Origin + Params.Rotation.RotateVector(FVector( \
		Random.FloatValue(-MaximumExtent.X, MaximumExtent.X), \
		Random.FloatValue(-MaximumExtent.Y, MaximumExtent.Y), \
		Random.FloatValue(-MaximumExtent.Z, MaximumExtent.Z)))
#define N_PICKER_ORIENTED_BOX_LOCATION_ROTATION(FloatValue) \
	Params.Origin + Params.Rotation.RotateVector(FVector( \
		Random.FloatValue(ChosenBox.Min.X, ChosenBox.Max.X), \
		Random.FloatValue(ChosenBox.Min.Y, ChosenBox.Max.Y), \
		Random.FloatValue(ChosenBox.Min.Z, ChosenBox.Max.Z)))

#if ENABLE_VISUAL_LOG
#define N_PICKER_ORIENTED_BOX_VALID_BOXES \
	TArray<FBox> ValidBoxes = Params.GetValidBoxes(); \
	if(CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		FTransform VLogTransform(Params.Rotation, Params.Origin); \
		FMatrix Matrix = VLogTransform.ToMatrixWithScale(); \
		for(int32 v = 0; v < ValidBoxes.Num(); v++) \
		{ \
			UE_VLOG_WIREOBOX(CachedWorld , LogNexusPicker, Verbose, ValidBoxes[v], Matrix, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		} \
	} \
	N_PICKER_ORIENTED_BOX_CUMULATIVE
#define N_PICKER_ORIENTED_BOX_VLOG(HasMinimumDimensions) \
	if(CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		FTransform VLogTransform(Params.Rotation, Params.Origin); \
		FMatrix Matrix = VLogTransform.ToMatrixWithScale(); \
		if(HasMinimumDimensions) \
		{ \
			UE_VLOG_WIREOBOX(CachedWorld, LogNexusPicker, Verbose, Params.GetMinimumAlignedBox(), Matrix, NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		} \
		UE_VLOG_WIREOBOX(CachedWorld, LogNexusPicker, Verbose, Params.GetMaximumAlignedBox(), Matrix, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		for (int32 i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(CachedWorld, LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else // !ENABLE_VISUAL_LOG
#define N_PICKER_ORIENTED_BOX_VLOG(HasMinimumDimensions)
#define N_PICKER_ORIENTED_BOX_VALID_BOXES \
	TArray<FBox> ValidBoxes = Params.GetValidBoxes(); \
	N_PICKER_ORIENTED_BOX_CUMULATIVE
#endif // ENABLE_VISUAL_LOG

// #SONARQUBE-DISABLE-CPP_S107 Lot of boilerplate code here
// Excluded from code duplication

#define RANDOM_FLOAT_RANGE FRandRange
void FNOrientedBoxPicker::Random(TArray<FVector>& OutLocations, const FNOrientedBoxPickerParams& Params)
{
	N_PICKER_RANDOM_NONDETERMINISTIC
	N_PICKER_ORIENTED_BOX_PREFIX
	if (bSimpleMode)
	{
		N_PICKER_ORIENTED_BOX_EXTENTS_SIMPLE
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
		}
		else
		{
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	else
	{
		N_PICKER_ORIENTED_BOX_VALID_BOXES
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
		}
		else
		{
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					OutLocations.Add(N_PICKER_ORIENTED_BOX_LOCATION_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					OutLocations.Add(N_PICKER_ORIENTED_BOX_LOCATION(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	N_PICKER_ORIENTED_BOX_VLOG(!bSimpleMode)
}
#undef RANDOM_FLOAT_RANGE

#define RANDOM_FLOAT_RANGE FRandRange
void FNOrientedBoxPicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNOrientedBoxPickerParams& Params)
{
	const FRandomStream Random(Seed);
	N_PICKER_ORIENTED_BOX_PREFIX
	if (bSimpleMode)
	{
		N_PICKER_ORIENTED_BOX_EXTENTS_SIMPLE
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
		}
		else
		{
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	else
	{
		N_PICKER_ORIENTED_BOX_VALID_BOXES
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
		}
		else
		{
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					OutLocations.Add(N_PICKER_ORIENTED_BOX_LOCATION_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					OutLocations.Add(N_PICKER_ORIENTED_BOX_LOCATION(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	N_PICKER_ORIENTED_BOX_VLOG(!bSimpleMode)
	Seed = Random.GetCurrentSeed();
}
#undef RANDOM_FLOAT_RANGE


#define RANDOM_FLOAT_RANGE FloatRange
void FNOrientedBoxPicker::Next(TArray<FVector>& OutLocations, FNMersenneTwister& Random, const FNOrientedBoxPickerParams& Params)
{
	N_PICKER_ORIENTED_BOX_PREFIX
	if (bSimpleMode)
	{
		N_PICKER_ORIENTED_BOX_EXTENTS_SIMPLE
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
		}
		else
		{
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	else
	{
		N_PICKER_ORIENTED_BOX_VALID_BOXES
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_ORIENTED_BOX_LOCATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
		}
		else
		{
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					OutLocations.Add(N_PICKER_ORIENTED_BOX_LOCATION_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_ORIENTED_BOX_VALID_BOXES_CHOICE(RANDOM_FLOAT_RANGE)
					OutLocations.Add(N_PICKER_ORIENTED_BOX_LOCATION(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	N_PICKER_ORIENTED_BOX_VLOG(!bSimpleMode)
}
#undef RANDOM_FLOAT_RANGE

// #SONARQUBE-ENABLE