// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NArcPicker.h"

#include "NavigationSystem.h"
#include "NPickerMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"


#define N_PICKER_ARC_PREFIX \
	const int OutLocationsStartIndex = OutLocations.Num(); \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count); \
	const float MaxDegrees = (Params.Degrees * 0.5f); \
	const float MinDegrees = -MaxDegrees;

#define N_PICKER_RADIAL_LOCATION(FloatRange) \
	const float RandomAngle = FMath::DegreesToRadians(FloatRange(MinDegrees, MaxDegrees)); \
	const float RandomDistance = FloatRange(Params.MinimumDistance, Params.MaximumDistance); \
	FVector Location = Params.Origin + Params.Rotation.RotateVector(FVector(FMath::Cos(RandomAngle), FMath::Sin(RandomAngle), 0) * RandomDistance);

#if ENABLE_VISUAL_LOG
#define N_PICKER_RADIAL_VLOG \
	if(Params.CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		const float MaxAngle = FMath::DegreesToRadians(MaxDegrees); \
		const float MinAngle = FMath::DegreesToRadians(MinDegrees); \
		const FVector InMinPoint = Params.Origin + Params.Rotation.RotateVector(FVector(FMath::Cos(MinAngle), FMath::Sin(MinAngle), 0) * Params.MinimumDistance); \
		const FVector OutMinPoint = Params.Origin + Params.Rotation.RotateVector(FVector(FMath::Cos(MinAngle), FMath::Sin(MinAngle), 0) * Params.MaximumDistance); \
		const FVector InMaxPoint = Params.Origin + Params.Rotation.RotateVector(FVector(FMath::Cos(MaxAngle), FMath::Sin(MaxAngle), 0) * Params.MinimumDistance); \
		const FVector OutMaxPoint = Params.Origin + Params.Rotation.RotateVector(FVector(FMath::Cos(MaxAngle), FMath::Sin(MaxAngle), 0) *Params.MaximumDistance); \
		UE_VLOG_SEGMENT(Params.CachedWorld, LogNexusPicker, Verbose, InMinPoint, OutMinPoint, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		UE_VLOG_SEGMENT(Params.CachedWorld, LogNexusPicker, Verbose, InMaxPoint, OutMaxPoint, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		for (int i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(Params.CachedWorld , LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else
#define N_PICKER_RADIAL_VLOG
#endif


#define RANDOM_FLOAT_RANGE FNRandom::Deterministic.FloatRange
void FNArcPicker::Next(TArray<FVector>& OutLocations, const FNArcPickerParams& Params)
{
	N_PICKER_ARC_PREFIX
	if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			N_PICKER_RADIAL_LOCATION(RANDOM_FLOAT_RANGE)
			N_IMPLEMENT_PICKER_PROJECTION_TRACE
			OutLocations.Add(Location);
		}
	}
	else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			N_PICKER_RADIAL_LOCATION(RANDOM_FLOAT_RANGE)
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
			OutLocations.Add(Location);
		}
	}
	else
	{
		for (int i = 0; i < Params.Count; i++)
		{
			N_PICKER_RADIAL_LOCATION(RANDOM_FLOAT_RANGE)
			OutLocations.Add(Location);
		}
	}
	N_PICKER_RADIAL_VLOG
}
#undef RANDOM_FLOAT_RANGE

#define RANDOM_FLOAT_RANGE FNRandom::NonDeterministic.FRandRange
void FNArcPicker::Random(TArray<FVector>& OutLocations, const FNArcPickerParams& Params)
{
	N_PICKER_ARC_PREFIX
	if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			N_PICKER_RADIAL_LOCATION(RANDOM_FLOAT_RANGE)
			N_IMPLEMENT_PICKER_PROJECTION_TRACE
			OutLocations.Add(Location);
		}
	}
	else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			N_PICKER_RADIAL_LOCATION(RANDOM_FLOAT_RANGE)
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
			OutLocations.Add(Location);
		}
	}
	else
	{
		for (int i = 0; i < Params.Count; i++)
		{
			N_PICKER_RADIAL_LOCATION(RANDOM_FLOAT_RANGE)
			OutLocations.Add(Location);
		}
	}
	N_PICKER_RADIAL_VLOG
}
#undef RANDOM_FLOAT_RANGE

#define RANDOM_FLOAT_RANGE RandomStream.FRandRange
void FNArcPicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNArcPickerParams& Params)
{
	const FRandomStream RandomStream(Seed);
	N_PICKER_ARC_PREFIX
	if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			N_PICKER_RADIAL_LOCATION(RANDOM_FLOAT_RANGE)
			N_IMPLEMENT_PICKER_PROJECTION_TRACE
			OutLocations.Add(Location);
		}
	}
	else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			N_PICKER_RADIAL_LOCATION(RANDOM_FLOAT_RANGE)
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
			OutLocations.Add(Location);
		}
	}
	else
	{
		for (int i = 0; i < Params.Count; i++)
		{
			N_PICKER_RADIAL_LOCATION(RANDOM_FLOAT_RANGE)
			OutLocations.Add(Location);
		}
	}
	N_PICKER_RADIAL_VLOG
	Seed = RandomStream.GetCurrentSeed();
}
#undef RANDOM_FLOAT_RANGE
