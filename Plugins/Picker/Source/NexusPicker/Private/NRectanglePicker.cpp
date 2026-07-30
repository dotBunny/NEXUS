// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NRectanglePicker.h"
#include "Algo/BinarySearch.h"
#include "NPickerMinimal.h"
#include "NPickerProjection.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_RECTANGLE_PREFIX \
	N_PICKER_PARAMS_WORLD_SAFETY \
	const int32 OutLocationsStartIndex = OutLocations.Num(); \
	const bool bSimpleMode = Params.MinimumDimensions.IsZero(); \
	const bool bHasRotation = !Params.Rotation.IsZero(); \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count);

#if ENABLE_VISUAL_LOG
#define N_PICKER_RECTANGLE_VLOG_DRAW(World, VerticesVariable, Color) \
	UE_VLOG_SEGMENT(World, LogNexusPicker, Verbose, VerticesVariable[0], VerticesVariable[1], Color, TEXT("")); \
	UE_VLOG_SEGMENT(World, LogNexusPicker, Verbose, VerticesVariable[1], VerticesVariable[2], Color, TEXT("")); \
	UE_VLOG_SEGMENT(World, LogNexusPicker, Verbose, VerticesVariable[2], VerticesVariable[3], Color, TEXT("")); \
	UE_VLOG_SEGMENT(World, LogNexusPicker, Verbose, VerticesVariable[3], VerticesVariable[0], Color, TEXT(""));
#define N_PICKER_RECTANGLE_VLOG_VERTICES(Origin, Dimensions, Rotation, VerticesVariable) \
	const float VerticesVariable##ExtentX = Dimensions.X * 0.5f; \
	const float VerticesVariable##ExtentY = Dimensions.Y * 0.5f; \
	TArray<FVector> VerticesVariable { \
		Origin + Rotation.RotateVector(FVector(-VerticesVariable##ExtentX, -VerticesVariable##ExtentY, 0)), \
		Origin + Rotation.RotateVector(FVector(-VerticesVariable##ExtentX, VerticesVariable##ExtentY, 0)), \
		Origin + Rotation.RotateVector(FVector(VerticesVariable##ExtentX, VerticesVariable##ExtentY, 0)), \
		Origin + Rotation.RotateVector(FVector(VerticesVariable##ExtentX, -VerticesVariable##ExtentY, 0)) \
	};
#define N_PICKER_RECTANGLE_VLOG_RANGES(Ranges) \
	if(CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		for(int32 v = 0; v < Ranges.Num(); v++) \
		{ \
			const FVector MinPoint = FVector(Ranges[v].X, Ranges[v].Y, 0); \
			const FVector MaxPoint = FVector(Ranges[v].Z, Ranges[v].W, 0); \
			TArray<FVector> RangeVertices; \
			RangeVertices.Reserve(4); \
			RangeVertices.Add(Params.Origin + Params.Rotation.RotateVector(MinPoint)); \
			RangeVertices.Add(Params.Origin + Params.Rotation.RotateVector(FVector(MaxPoint.X, MinPoint.Y, 0.f))); \
			RangeVertices.Add(Params.Origin + Params.Rotation.RotateVector(MaxPoint)); \
			RangeVertices.Add(Params.Origin + Params.Rotation.RotateVector(FVector(MinPoint.X, MaxPoint.Y, 0.f))); \
			N_PICKER_RECTANGLE_VLOG_DRAW(CachedWorld, RangeVertices, NEXUS::Picker::VLog::OuterColor) \
		} \
	}
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
#define N_PICKER_RECTANGLE_VLOG_RANGES(Ranges)
#define N_PICKER_RECTANGLE_VLOG(HasMinimumDimensions)
#endif // ENABLE_VISUAL_LOG

namespace
{
	// Single source of truth for rectangle point generation. Random/Tracked/Next differ only in how a float
	// in [Min,Max] is drawn, so that is the one parameter: Rand(Min, Max) -> float.
	template <typename FRandFloat>
	void GenerateRectanglePoints(TArray<FVector>& OutLocations, const FNRectanglePickerParams& Params, FRandFloat&& Rand)
	{
		N_PICKER_RECTANGLE_PREFIX

		const float ExtentX = Params.MaximumDimensions.X * 0.5f;
		const float ExtentY = Params.MaximumDimensions.Y * 0.5f;

		// Shell mode: select a sub-range weighted by area. Ranges packed as (MinX, MinY, MaxX, MaxY). Built
		// once, queried per point.
		TArray<FVector4> ValidRanges;
		TArray<double> CumulativeAreas;
		double TotalArea = 0.0;
		if (!bSimpleMode)
		{
			ValidRanges = Params.GetValidRanges();
			CumulativeAreas.Reserve(ValidRanges.Num());
			for (const FVector4& CumulativeRange : ValidRanges)
			{
				TotalArea += (CumulativeRange.Z - CumulativeRange.X) * (CumulativeRange.W - CumulativeRange.Y);
				CumulativeAreas.Add(TotalArea);
			}
			N_PICKER_RECTANGLE_VLOG_RANGES(ValidRanges)
		}

		FNPickerProjection::Emit(OutLocations, CachedWorld, Params, [&]() -> FVector
		{
			FVector Local;
			if (bSimpleMode)
			{
				Local = FVector(Rand(-ExtentX, ExtentX), Rand(-ExtentY, ExtentY), 0.f);
			}
			else
			{
				const double AreaPick = Rand(0.f, 1.f) * TotalArea;
				const int32 ChosenIndex = FMath::Min(Algo::LowerBound(CumulativeAreas, AreaPick), ValidRanges.Num() - 1);
				const FVector4 ChosenRange = ValidRanges[ChosenIndex];
				Local = FVector(Rand(ChosenRange.X, ChosenRange.Z), Rand(ChosenRange.Y, ChosenRange.W), 0.f);
			}
			return Params.Origin + (bHasRotation ? Params.Rotation.RotateVector(Local) : Local);
		});

		N_PICKER_RECTANGLE_VLOG(!bSimpleMode)
	}
}

void FNRectanglePicker::Random(TArray<FVector>& OutLocations, const FNRectanglePickerParams& Params)
{
	N_PICKER_RANDOM_NONDETERMINISTIC
	GenerateRectanglePoints(OutLocations, Params, [&Random](const float Min, const float Max) { return Random.FRandRange(Min, Max); });
}

void FNRectanglePicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNRectanglePickerParams& Params)
{
	const FRandomStream Random(Seed);
	GenerateRectanglePoints(OutLocations, Params, [&Random](const float Min, const float Max) { return Random.FRandRange(Min, Max); });
	Seed = Random.GetCurrentSeed();
}

void FNRectanglePicker::Next(TArray<FVector>& OutLocations, FNMersenneTwister& Random, const FNRectanglePickerParams& Params)
{
	GenerateRectanglePoints(OutLocations, Params, [&Random](const float Min, const float Max) { return Random.FloatRange(Min, Max); });
}
