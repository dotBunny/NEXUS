// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NRectanglePicker.h"
#include "NavigationSystem.h"
#include "NPickerMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_RECTANGLE_PREFIX \
	const int OutLocationsStartIndex = OutLocations.Num(); \
	const bool bSimpleMode = Params.MinimumDimensions.IsZero(); \
	const bool bHasRotation = !Params.Rotation.IsZero(); \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count);
#define N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RandomIndex) \
	FVector4 ChosenRange = ValidRanges[RandomIndex(0,ValidRanges.Num()-1)];
#define N_PICKER_RECTANGLE_EXTENTS_SIMPLE \
	const float ExtentX = Params.MaximumDimensions.X * 0.5f; \
	const float ExtentY = Params.MaximumDimensions.Y * 0.5f;
#define N_PICKER_RECTANGLE_LOCATION(FloatValue) \
	Params.Origin + FVector(FloatValue(ChosenRange.X, ChosenRange.Z), FloatValue(ChosenRange.Y, ChosenRange.W), 0.f)
#define N_PICKER_RECTANGLE_LOCATION_ROTATION(FloatValue) \
	Params.Origin + Params.Rotation.RotateVector(FVector(FloatValue(ChosenRange.X, ChosenRange.Z), FloatValue(ChosenRange.Y, ChosenRange.W), 0.f))
#define N_PICKER_RECTANGLE_LOCATION_SIMPLE(FloatValue) \
	Params.Origin + FVector ( FloatValue(-ExtentX, ExtentX), FloatValue(-ExtentY, ExtentY), 0.f)
#define N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(FloatValue) \
	Params.Origin + Params.Rotation.RotateVector(FVector(FloatValue(-ExtentX, ExtentX), FloatValue(-ExtentY, ExtentY), 0.f))

#if ENABLE_VISUAL_LOG
#define N_PICKER_RECTANGLE_VALID_RANGES \
	TArray<FVector4> ValidRanges = Params.GetValidRanges(); \
	if(Params.CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		for(int v = 0; v < ValidRanges.Num(); v++) \
		{ \
			const FVector MinPoint = FVector(ValidRanges[v].X, ValidRanges[v].Y, 0); \
			const FVector MaxPoint = FVector(ValidRanges[v].Z, ValidRanges[v].W, 0); \
			TArray<FVector> RangeVertices; \
			RangeVertices.Reserve(4); \
			RangeVertices.Add(Params.Origin + Params.Rotation.RotateVector(MinPoint)); \
			RangeVertices.Add(Params.Origin + Params.Rotation.RotateVector(FVector(MaxPoint.X, MinPoint.Y, 0.f))); \
			RangeVertices.Add(Params.Origin + Params.Rotation.RotateVector(MaxPoint)); \
			RangeVertices.Add(Params.Origin + Params.Rotation.RotateVector(FVector(MinPoint.X, MaxPoint.Y, 0.f))); \
			N_PICKER_RECTANGLE_VLOG_DRAW(Params.CachedWorld, RangeVertices, NEXUS::Picker::VLog::OuterColor) \
		} \
	}
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
	if(Params.CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		if(HasMinimumDimensions) \
		{ \
			N_PICKER_RECTANGLE_VLOG_VERTICES(Params.Origin, Params.MinimumDimensions, Params.Rotation, MinimumPoints) \
			N_PICKER_RECTANGLE_VLOG_DRAW(Params.CachedWorld, MinimumPoints, NEXUS::Picker::VLog::InnerColor) \
		} \
		N_PICKER_RECTANGLE_VLOG_VERTICES(Params.Origin, Params.MaximumDimensions, Params.Rotation, MaximumPoints) \
		N_PICKER_RECTANGLE_VLOG_DRAW(Params.CachedWorld, MaximumPoints, NEXUS::Picker::VLog::OuterColor) \
		for (int i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(Params.CachedWorld , LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else // ENABLE_VISUAL_LOG
#define N_PICKER_RECTANGLE_VLOG(HasMinimumDimensions)
#define N_PICKER_RECTANGLE_VALID_RANGES \
	TArray<FVector4> ValidRanges = Params.GetValidRanges();
#endif // ENABLE_VISUAL_LOG

// #SONARQUBE-DISABLE-CPP_S107 Lot of boilerplate code here
// Excluded from code duplication

#define RANDOM_INDEX FNRandom::Deterministic.IntegerRange
#define RANDOM_FLOAT_RANGE FNRandom::Deterministic.FloatRange
#define RANDOM_BOOL FNRandom::Deterministic.Bool()
void FNRectanglePicker::Next(TArray<FVector>& OutLocations, const FNRectanglePickerParams& Params)
{
	N_PICKER_RECTANGLE_PREFIX
	if (bSimpleMode)
	{
		N_PICKER_RECTANGLE_EXTENTS_SIMPLE
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
		}
		else
		{
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	else
	{
		N_PICKER_RECTANGLE_VALID_RANGES
		
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
		}
		else
		{
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	N_PICKER_RECTANGLE_VLOG(!bSimpleMode)
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_INDEX

#define RANDOM_FLOAT_RANGE FNRandom::NonDeterministic.FRandRange
#define RANDOM_INDEX FNRandom::NonDeterministic.RandRange
void FNRectanglePicker::Random(TArray<FVector>& OutLocations, const FNRectanglePickerParams& Params)
{
	N_PICKER_RECTANGLE_PREFIX
	if (bSimpleMode)
	{
		N_PICKER_RECTANGLE_EXTENTS_SIMPLE
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
		}
		else
		{
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	else
	{		
		N_PICKER_RECTANGLE_VALID_RANGES
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
		}
		else
		{
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	N_PICKER_RECTANGLE_VLOG(!bSimpleMode)
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_INDEX

#define RANDOM_FLOAT_RANGE RandomStream.FRandRange
#define RANDOM_INDEX RandomStream.RandRange
void FNRectanglePicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNRectanglePickerParams& Params)
{
	const FRandomStream RandomStream(Seed);
	N_PICKER_RECTANGLE_PREFIX
	if (bSimpleMode)
	{
		N_PICKER_RECTANGLE_EXTENTS_SIMPLE
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
		}
		else
		{
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_SIMPLE(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	else
	{
		N_PICKER_RECTANGLE_VALID_RANGES
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
		}
		else if (Params.ProjectionMode == ENPickerProjectionMode::NearestNavMeshV1 && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE);
					N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
		}
		else
		{
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE));
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					N_PICKER_RECTANGLE_VALID_RANGES_CHOICE(RANDOM_INDEX)
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE));
				}
			}
		}
	}
	N_PICKER_RECTANGLE_VLOG(!bSimpleMode)
	Seed = RandomStream.GetCurrentSeed();
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_INDEX

// #SONARQUBE-ENABLE
