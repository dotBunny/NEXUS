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
#define N_PICKER_RECTANGLE_EXTENTS \
	const float MaximumExtentX = Params.MaximumDimensions.X * 0.5f; \
	const float MaximumExtentY = Params.MaximumDimensions.Y * 0.5f; \
	const float MinimumExtentX = Params.MinimumDimensions.X * 0.5f; \
	const float MinimumExtentY = Params.MinimumDimensions.Y * 0.5f;
#define N_PICKER_RECTANGLE_EXTENTS_SIMPLE \
	const float ExtentX = Params.MaximumDimensions.X * 0.5f; \
	const float ExtentY = Params.MaximumDimensions.Y * 0.5f;
#define N_PICKER_RECTANGLE_LOCATION(FloatValue, BooleanValue) \
	Params.Origin + FVector( \
		BooleanValue ? FloatValue(-MinimumExtentX, -MaximumExtentX) : FloatValue(MinimumExtentX, MaximumExtentX), \
		BooleanValue ? FloatValue(-MinimumExtentY, -MaximumExtentY) : FloatValue(MinimumExtentY, MaximumExtentY), \
		0.f)
#define N_PICKER_RECTANGLE_LOCATION_ROTATION(FloatValue, BooleanValue) \
	Params.Origin + Params.Rotation.RotateVector(FVector( \
		BooleanValue ? FloatValue(-MinimumExtentX, -MaximumExtentX) : FloatValue(MinimumExtentX, MaximumExtentX), \
		BooleanValue ? FloatValue(-MinimumExtentY, -MaximumExtentY) : FloatValue(MinimumExtentY, MaximumExtentY), \
		0.f))
#define N_PICKER_RECTANGLE_LOCATION_SIMPLE(FloatValue) \
	Params.Origin + FVector ( FloatValue(-ExtentX, ExtentX), FloatValue(-ExtentY, ExtentY), 0.f)
#define N_PICKER_RECTANGLE_LOCATION_SIMPLE_ROTATION(FloatValue) \
	Params.Origin + Params.Rotation.RotateVector(FVector(FloatValue(-ExtentX, ExtentX), FloatValue(-ExtentY, ExtentY), 0.f))

#if ENABLE_VISUAL_LOG
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
			N_PICKER_RECTANGLE_VLOG_DRAW(Params.CachedWorld, MinimumPoints, NEXUS::Picker::VLog::InnerColor); \
		} \
		N_PICKER_RECTANGLE_VLOG_VERTICES(Params.Origin, Params.MaximumDimensions, Params.Rotation, MaximumPoints) \
		N_PICKER_RECTANGLE_VLOG_DRAW(Params.CachedWorld, MaximumPoints, NEXUS::Picker::VLog::OuterColor); \
		for (int i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(Params.CachedWorld , LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else
#define N_PICKER_RECTANGLE_VLOG(HasMinimumDimensions)
#endif

// #SONARQUBE-DISABLE-CPP_S107 Lot of boilerplate code here
// Excluded from code duplication

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
		N_PICKER_RECTANGLE_EXTENTS
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
					N_IMPLEMENT_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
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
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
					N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
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
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL));
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL));
				}
			}
		}
	}
	N_PICKER_RECTANGLE_VLOG(!bSimpleMode)
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_BOOL

#define RANDOM_FLOAT_RANGE FNRandom::NonDeterministic.FRandRange
#define RANDOM_BOOL FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f
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
		N_PICKER_RECTANGLE_EXTENTS
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
					N_IMPLEMENT_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
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
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
					N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
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
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL));
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL));
				}
			}
		}
	}
	N_PICKER_RECTANGLE_VLOG(!bSimpleMode)
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_BOOL

#define RANDOM_FLOAT_RANGE RandomStream.FRandRange
#define RANDOM_BOOL RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f
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
		N_PICKER_RECTANGLE_EXTENTS
		if (Params.ProjectionMode == ENPickerProjectionMode::Trace && Params.CachedWorld != nullptr)
		{
			N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX
			if (bHasRotation)
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
					N_IMPLEMENT_PICKER_PROJECTION_TRACE
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
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
					FVector Location = N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
					N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1
					OutLocations.Add(Location);
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					FVector Location = N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL);
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
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION_ROTATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL));
				}
			}
			else
			{
				for (int i = 0; i < Params.Count; i++)
				{
					OutLocations.Add(N_PICKER_RECTANGLE_LOCATION(RANDOM_FLOAT_RANGE, RANDOM_BOOL));
				}
			}
		}
	}
	N_PICKER_RECTANGLE_VLOG(!bSimpleMode)
	Seed = RandomStream.GetCurrentSeed();
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_BOOL

// #SONARQUBE-ENABLE
