// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSplinePicker.h"
#include "NavigationSystem.h"
#include "NPickerMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_SPLINE_PREFIX \
	const int OutLocationsStartIndex = OutLocations.Num(); \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count);
#define N_PICKER_SPLINE_LOCATION(FloatRange) \
	Params.SplineComponent->GetWorldLocationAtTime(FloatRange(0, Params.SplineComponent->Duration))
#if ENABLE_VISUAL_LOG
#define N_IMPLEMENT_VLOG_SPLINE \
	if(Params.CachedWorld != nullptr && FVisualLogger::IsRecording()) { \
		TArray<FVector> SplinePoints; \
		const float SplineLength = Params.SplineComponent->GetSplineLength(); \
		const int32 NumSegments = FMath::Max(20, FMath::RoundToInt(SplineLength / 20.0f)); \
		const float DistancePerSegment = SplineLength / NumSegments; \
		SplinePoints.Reserve(NumSegments); \
		for (int32 i = 0; i <= NumSegments; ++i) \
		{ \
			const float Distance = DistancePerSegment * i; \
			const FVector Point = Params.SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World); \
			SplinePoints.Add(Point); \
		} \
		for (int32 i = 0; i < NumSegments; ++i) \
		{ \
			UE_VLOG_SEGMENT(Params.CachedWorld, LogNexusPicker, Verbose, SplinePoints[i], SplinePoints[i+1], NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		} \
		if(Params.SplineComponent->IsClosedLoop()) \
		{ \
			UE_VLOG_SEGMENT(Params.CachedWorld, LogNexusPicker, Verbose, SplinePoints[NumSegments-1], SplinePoints[0], NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		} \
		for (int i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(Params.CachedWorld , LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else // ENABLE_VISUAL_LOG
#define N_IMPLEMENT_VLOG_SPLINE
#endif // ENABLE_VISUAL_LOG

// #SONARQUBE-DISABLE-CPP_S107 Lot of boilerplate code here
// Excluded from code duplication

#define RANDOM_FLOAT_RANGE FNRandom::Deterministic.FloatRange
void FNSplinePicker::Next(TArray<FVector>& OutLocations, const FNSplinePickerParams& Params)
{
	N_PICKER_SPLINE_PREFIX
	if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			FVector Location = N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE);
			N_IMPLEMENT_PICKER_PROJECTION_TRACE
			OutLocations.Add(Location);
		}
	}
	else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			FVector Location = N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE);
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
			OutLocations.Add(Location);
		}
	}
	else
	{
		for (int i = 0; i < Params.Count; i++)
		{
			OutLocations.Add(N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE));
		}
	}
	N_IMPLEMENT_VLOG_SPLINE
}
#undef RANDOM_FLOAT_RANGE

#define RANDOM_FLOAT_RANGE FNRandom::NonDeterministic.FRandRange
void FNSplinePicker::Random(TArray<FVector>& OutLocations, const FNSplinePickerParams& Params)
{
	N_PICKER_SPLINE_PREFIX
	if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			FVector Location = N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE);
			N_IMPLEMENT_PICKER_PROJECTION_TRACE
			OutLocations.Add(Location);
		}
	}
	else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			FVector Location = N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE);
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
			OutLocations.Add(Location);
		}
	}
	else
	{
		for (int i = 0; i < Params.Count; i++)
		{
			OutLocations.Add(N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE));
		}
	}
	N_IMPLEMENT_VLOG_SPLINE
}
#undef RANDOM_FLOAT_RANGE

#define RANDOM_FLOAT_RANGE RandomStream.FRandRange
void FNSplinePicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNSplinePickerParams& Params)
{
	const FRandomStream RandomStream(Seed);
	N_PICKER_SPLINE_PREFIX
	if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			FVector Location = N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE);
			N_IMPLEMENT_PICKER_PROJECTION_TRACE
			OutLocations.Add(Location);
		}
	}
	else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
	{
		N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
		for (int i = 0; i < Params.Count; i++)
		{
			FVector Location = N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE);
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
			OutLocations.Add(Location);
		}
	}
	else
	{
		for (int i = 0; i < Params.Count; i++)
		{
			OutLocations.Add(N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE));
		}
	}
	N_IMPLEMENT_VLOG_SPLINE
	Seed = RandomStream.GetCurrentSeed();
}
#undef RANDOM_FLOAT_RANGE

// #SONARQUBE-ENABLE