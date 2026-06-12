// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSplinePicker.h"
#include "NavigationSystem.h"
#include "NPickerMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"
#include "Math/NMersenneTwister.h"

#define N_PICKER_SPLINE_PREFIX \
	N_PICKER_PARAMS_WORLD_SAFETY \
	const int32 OutLocationsStartIndex = OutLocations.Num(); \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count); \
	if (!IsValid(Params.SplineComponent)) \
	{ \
		for (int32 i = 0; i < Params.Count; i++) \
		{ \
			OutLocations.Add(FVector::ZeroVector); \
		} \
		UE_LOG(LogNexusPicker, Error, TEXT("Unable to pick points on spline as either no spline component was provided, or it was invalid. Defaulting to origin points.")) \
		return; \
	}
#define N_PICKER_SPLINE_LOCATION(FloatRange) \
	Params.SplineComponent->GetLocationAtDistanceAlongSpline( \
		Random.FloatRange(0.f, Params.SplineComponent->GetSplineLength()), ESplineCoordinateSpace::World)
#if ENABLE_VISUAL_LOG
#define N_VLOG_SPLINE \
	if(CachedWorld != nullptr && FVisualLogger::IsRecording()) { \
		TArray<FVector> SplinePoints; \
		const float SplineLength = Params.SplineComponent->GetSplineLength(); \
		const int32 NumSegments = FMath::Max(20, FMath::RoundToInt(SplineLength / 20.0f)); \
		const float DistancePerSegment = SplineLength / NumSegments; \
		SplinePoints.Reserve(NumSegments + 1); \
		for (int32 i = 0; i <= NumSegments; ++i) \
		{ \
			const float Distance = DistancePerSegment * i; \
			const FVector Point = Params.SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World); \
			SplinePoints.Add(Point); \
		} \
		for (int32 i = 0; i < NumSegments; ++i) \
		{ \
			UE_VLOG_SEGMENT(CachedWorld, LogNexusPicker, Verbose, SplinePoints[i], SplinePoints[i+1], NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		} \
		for (int32 i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(CachedWorld , LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else // !ENABLE_VISUAL_LOG
#define N_VLOG_SPLINE
#endif // ENABLE_VISUAL_LOG

// #SONARQUBE-DISABLE-CPP_S107 Lot of boilerplate code here
// Excluded from code duplication

#define RANDOM_FLOAT_RANGE FRandRange
void FNSplinePicker::Random(TArray<FVector>& OutLocations, const FNSplinePickerParams& Params)
{
	N_PICKER_RANDOM_NONDETERMINISTIC
	N_PICKER_SPLINE_PREFIX
	if (Params.ProjectionMode == ENPickerProjectionMode::Trace && CachedWorld != nullptr)
	{
		N_PICKER_PROJECTION_TRACE_PREFIX
		for (int32 i = 0; i < Params.Count; i++)
		{
			FVector Location = N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE);
			N_PICKER_PROJECTION_TRACE
			OutLocations.Add(Location);
		}
	}
	else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && CachedWorld != nullptr)
	{
		N_PICKER_PROJECTION_NAVMESH_V1_PREFIX
		for (int32 i = 0; i < Params.Count; i++)
		{
			FVector Location = N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE);
			N_PICKER_PROJECTION_NAVMESH_V1
			OutLocations.Add(Location);
		}
	}
	else
	{
		for (int32 i = 0; i < Params.Count; i++)
		{
			OutLocations.Add(N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE));
		}
	}
	N_VLOG_SPLINE
}
#undef RANDOM_FLOAT_RANGE

#define RANDOM_FLOAT_RANGE FRandRange
void FNSplinePicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNSplinePickerParams& Params)
{
	const FRandomStream Random(Seed);
	N_PICKER_SPLINE_PREFIX
	if (Params.ProjectionMode == ENPickerProjectionMode::Trace && CachedWorld != nullptr)
	{
		N_PICKER_PROJECTION_TRACE_PREFIX
		for (int32 i = 0; i < Params.Count; i++)
		{
			FVector Location = N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE);
			N_PICKER_PROJECTION_TRACE
			OutLocations.Add(Location);
		}
	}
	else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && CachedWorld != nullptr)
	{
		N_PICKER_PROJECTION_NAVMESH_V1_PREFIX
		for (int32 i = 0; i < Params.Count; i++)
		{
			FVector Location = N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE);
			N_PICKER_PROJECTION_NAVMESH_V1
			OutLocations.Add(Location);
		}
	}
	else
	{
		for (int32 i = 0; i < Params.Count; i++)
		{
			OutLocations.Add(N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE));
		}
	}
	N_VLOG_SPLINE
	Seed = Random.GetCurrentSeed();
}
#undef RANDOM_FLOAT_RANGE

#define RANDOM_FLOAT_RANGE FloatRange
void FNSplinePicker::Next(TArray<FVector>& OutLocations, FNMersenneTwister& Random, const FNSplinePickerParams& Params)
{
	N_PICKER_SPLINE_PREFIX
	if (Params.ProjectionMode == ENPickerProjectionMode::Trace && CachedWorld != nullptr)
	{
		N_PICKER_PROJECTION_TRACE_PREFIX
		for (int32 i = 0; i < Params.Count; i++)
		{
			FVector Location = N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE);
			N_PICKER_PROJECTION_TRACE
			OutLocations.Add(Location);
		}
	}
	else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && CachedWorld != nullptr)
	{
		N_PICKER_PROJECTION_NAVMESH_V1_PREFIX
		for (int32 i = 0; i < Params.Count; i++)
		{
			FVector Location = N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE);
			N_PICKER_PROJECTION_NAVMESH_V1
			OutLocations.Add(Location);
		}
	}
	else
	{
		for (int32 i = 0; i < Params.Count; i++)
		{
			OutLocations.Add(N_PICKER_SPLINE_LOCATION(RANDOM_FLOAT_RANGE));
		}
	}
	N_VLOG_SPLINE
}
#undef RANDOM_FLOAT_RANGE

// #SONARQUBE-ENABLE