// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NRectanglePicker.h"
#include "Algo/BinarySearch.h"
#include "NavigationSystem.h"
#include "NPickerMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_RECTANGLE_PREFIX \
	N_PICKER_PARAMS_WORLD_SAFETY \
	const int32 OutLocationsStartIndex = OutLocations.Num(); \
	const bool bSimpleMode = Params.MinimumDimensions.IsZero(); \
	const bool bHasRotation = !Params.Rotation.IsZero(); \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count);
#define N_PICKER_RECTANGLE_CUMULATIVE \
	TArray<double> CumulativeAreas; \
	CumulativeAreas.Reserve(ValidRanges.Num()); \
	double TotalArea = 0.0; \
	for (const FVector4& CumulativeRange : ValidRanges) \
	{ \
		TotalArea += (CumulativeRange.Z - CumulativeRange.X) * (CumulativeRange.W - CumulativeRange.Y); \
		CumulativeAreas.Add(TotalArea); \
	}
#define N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(FloatValue) \
	const double AreaPick = Random.FloatValue(0.f, 1.f) * TotalArea; \
	const int32 ChosenIndex = FMath::Min(Algo::LowerBound(CumulativeAreas, AreaPick), ValidRanges.Num() - 1); \
	const FVector4 ChosenRange = ValidRanges[ChosenIndex];
#define N_PICKER_RECTANGLE_EXTENTS_SIMPLE \
	const float ExtentX = Params.MaximumDimensions.X * 0.5f; \
	const float ExtentY = Params.MaximumDimensions.Y * 0.5f;
#define N_PICKER_RECTANGLE_LOCATION(FloatValue) \
	Params.Origin + FVector(Random.FloatValue(ChosenRange.X, ChosenRange.Z), Random.FloatValue(ChosenRange.Y, ChosenRange.W), 0.f)
#define N_PICKER_RECTANGLE_LOCATION_ROTATION(FloatValue) \
	Params.Origin + Params.Rotation.RotateVector(FVector(Random.FloatValue(ChosenRange.X, ChosenRange.Z), Random.FloatValue(ChosenRange.Y, ChosenRange.W), 0.f))
#define N_PICKER_RECTANGLE_LOCATION_SIMPLE(FloatValue) \
	Params.Origin + FVector (Random.FloatValue(-ExtentX, ExtentX), Random.FloatValue(-ExtentY, ExtentY), 0.f)
#define N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(FloatValue) \
	Params.Origin + Params.Rotation.RotateVector(FVector(Random.FloatValue(-ExtentX, ExtentX), Random.FloatValue(-ExtentY, ExtentY), 0.f))

#if ENABLE_VISUAL_LOG
#define N_PICKER_RECTANGLE_VALID_RANGES \
	TArray<FVector4> ValidRanges = Params.GetValidRanges(); \
	if(CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		for(int32 v = 0; v < ValidRanges.Num(); v++) \
		{ \
			const FVector MinPoint = FVector(ValidRanges[v].X, ValidRanges[v].Y, 0); \
			const FVector MaxPoint = FVector(ValidRanges[v].Z, ValidRanges[v].W, 0); \
			TArray<FVector> RangeVertices; \
			RangeVertices.Reserve(4); \
			RangeVertices.Add(Params.Origin + Params.Rotation.RotateVector(MinPoint)); \
			RangeVertices.Add(Params.Origin + Params.Rotation.RotateVector(FVector(MaxPoint.X, MinPoint.Y, 0.f))); \
			RangeVertices.Add(Params.Origin + Params.Rotation.RotateVector(MaxPoint)); \
			RangeVertices.Add(Params.Origin + Params.Rotation.RotateVector(FVector(MinPoint.X, MaxPoint.Y, 0.f))); \
			N_PICKER_RECTANGLE_VLOG_DRAW(CachedWorld, RangeVertices, NEXUS::Picker::VLog::OuterColor) \
		} \
	} \
	N_PICKER_RECTANGLE_CUMULATIVE
#define N_PICKER_RECTANGLE_VLOG_VERTICES(Origin, Dimensions, Rotation, VerticesVariable) \
	const float VerticesVariable##ExtentX = Dimensions.X * 0.5f; \
	const float VerticesVariable##ExtentY = Dimensions.Y * 0.5f; \
	TArray<FVector> VerticesVariable { \
		Origin + Rotation.RotateVector(FVector(-VerticesVariable##ExtentX, -VerticesVariable##ExtentY, 0)), \
		Origin + Rotation.RotateVector(FVector(-VerticesVariable##ExtentX, VerticesVariable##ExtentY, 0)), \
		Origin + Rotation.RotateVector(FVector(VerticesVariable##ExtentX, VerticesVariable##ExtentY, 0)), \
		Origin + Rotation.RotateVector(FVector(VerticesVariable##ExtentX, -VerticesVariable##ExtentY, 0)) \
	};
#define N_PICKER_RECTANGLE_VLOG_DRAW(World, VerticesVariable, Color) \
	UE_VLOG_SEGMENT(World, LogNexusPicker, Verbose, VerticesVariable[0], VerticesVariable[1], Color, TEXT("")); \
	UE_VLOG_SEGMENT(World, LogNexusPicker, Verbose, VerticesVariable[1], VerticesVariable[2], Color, TEXT("")); \
	UE_VLOG_SEGMENT(World, LogNexusPicker, Verbose, VerticesVariable[2], VerticesVariable[3], Color, TEXT("")); \
	UE_VLOG_SEGMENT(World, LogNexusPicker, Verbose, VerticesVariable[3], VerticesVariable[0], Color, TEXT(""));
#define N_PICKER_RECTANGLE_VLOG(HasMinimumDimensions) \
	if(CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		if(HasMinimumDimensions) \
		{ \
			N_PICKER_RECTANGLE_VLOG_VERTICES(Params.Origin, Params.MinimumDimensions, Params.Rotation, MinimumPoints) \
			N_PICKER_RECTANGLE_VLOG_DRAW(CachedWorld, MinimumPoints, NEXUS::Picker::VLog::InnerColor) \
		} \
		N_PICKER_RECTANGLE_VLOG_VERTICES(Params.Origin, Params.MaximumDimensions, Params.Rotation, MaximumPoints) \
		N_PICKER_RECTANGLE_VLOG_DRAW(CachedWorld, MaximumPoints, NEXUS::Picker::VLog::OuterColor) \
		for (int32 i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(CachedWorld , LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else // !ENABLE_VISUAL_LOG
#define N_PICKER_RECTANGLE_VLOG(HasMinimumDimensions)
#define N_PICKER_RECTANGLE_VALID_RANGES \
	TArray<FVector4> ValidRanges = Params.GetValidRanges(); \
	N_PICKER_RECTANGLE_CUMULATIVE
#endif // ENABLE_VISUAL_LOG

// #SONARQUBE-DISABLE-CPP_S107 Lot of boilerplate code here
// Excluded from code duplication

#define RANDOM_FLOAT_RANGE FRandRange
void FNRectanglePicker::Random(TArray<FVector>& OutLocations, const FNRectanglePickerParams& Params)
{
	N_PICKER_RANDOM_NONDETERMINISTIC
	N_PICKER_RECTANGLE_PREFIX
	if (bSimpleMode)
	{
		N_PICKER_RECTANGLE_EXTENTS_SIMPLE
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
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
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
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
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	else
	{		
		N_PICKER_RECTANGLE_VALID_RANGES
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE);
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
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE);
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
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	N_PICKER_RECTANGLE_VLOG(!bSimpleMode)
}
#undef RANDOM_FLOAT_RANGE

#define RANDOM_FLOAT_RANGE FRandRange
void FNRectanglePicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNRectanglePickerParams& Params)
{
	const FRandomStream Random(Seed);
	N_PICKER_RECTANGLE_PREFIX
	if (bSimpleMode)
	{
		N_PICKER_RECTANGLE_EXTENTS_SIMPLE
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
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
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
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
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	else
	{
		N_PICKER_RECTANGLE_VALID_RANGES
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE);
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
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE);
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
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	N_PICKER_RECTANGLE_VLOG(!bSimpleMode)
	Seed = Random.GetCurrentSeed();
}

#undef RANDOM_FLOAT_RANGE

#define RANDOM_FLOAT_RANGE FloatRange
void FNRectanglePicker::Next(TArray<FVector>& OutLocations, FNMersenneTwister& Random, const FNRectanglePickerParams& Params)
{
	N_PICKER_RECTANGLE_PREFIX
	if (bSimpleMode)
	{
		N_PICKER_RECTANGLE_EXTENTS_SIMPLE
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
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
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
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
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	else
	{
		N_PICKER_RECTANGLE_VALID_RANGES
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && CachedWorld != nullptr)
		{
			N_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE);
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
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE);
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
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int32 i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_FLOAT_RANGE)
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	N_PICKER_RECTANGLE_VLOG(!bSimpleMode)
}

#undef RANDOM_FLOAT_RANGE

// #SONARQUBE-ENABLE
