// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NOrientedBoxPicker.h"
#include "Algo/BinarySearch.h"
#include "NPickerMinimal.h"
#include "NPickerProjection.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_ORIENTED_BOX_PREFIX \
	N_PICKER_PARAMS_WORLD_SAFETY \
	const int32 OutLocationsStartIndex = OutLocations.Num(); \
	const bool bSimpleMode = Params.MinimumDimensions.IsZero(); \
	const bool bHasRotation = !Params.Rotation.IsZero(); \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count);

#if ENABLE_VISUAL_LOG
#define N_PICKER_ORIENTED_BOX_VLOG_BOXES(Boxes) \
	if(CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		FTransform VLogTransform(Params.Rotation, Params.Origin); \
		FMatrix Matrix = VLogTransform.ToMatrixWithScale(); \
		for(int32 v = 0; v < Boxes.Num(); v++) \
		{ \
			UE_VLOG_WIREOBOX(CachedWorld , LogNexusPicker, Verbose, Boxes[v], Matrix, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		} \
	}
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
#define N_PICKER_ORIENTED_BOX_VLOG_BOXES(Boxes)
#define N_PICKER_ORIENTED_BOX_VLOG(HasMinimumDimensions)
#endif // ENABLE_VISUAL_LOG

namespace
{
	// Single source of truth for oriented-box point generation. Random/Tracked/Next differ only in how a
	// float in [Min,Max] is drawn, so that is the one parameter: Rand(Min, Max) -> float.
	template <typename FRandFloat>
	void GenerateOrientedBoxPoints(TArray<FVector>& OutLocations, const FNOrientedBoxPickerParams& Params, FRandFloat&& Rand)
	{
		N_PICKER_ORIENTED_BOX_PREFIX

		const FVector MaximumExtent = 0.5f * Params.MaximumDimensions;

		// Shell mode: select a sub-box weighted by volume. Built once, queried per point.
		TArray<FBox> ValidBoxes;
		TArray<double> CumulativeVolumes;
		double TotalVolume = 0.0;
		if (!bSimpleMode)
		{
			ValidBoxes = Params.GetValidBoxes();
			CumulativeVolumes.Reserve(ValidBoxes.Num());
			for (const FBox& CumulativeBox : ValidBoxes)
			{
				TotalVolume += CumulativeBox.GetVolume();
				CumulativeVolumes.Add(TotalVolume);
			}
			N_PICKER_ORIENTED_BOX_VLOG_BOXES(ValidBoxes)
		}

		FNPickerProjection::Emit(OutLocations, CachedWorld, Params, [&]() -> FVector
		{
			FVector Local;
			if (bSimpleMode)
			{
				Local = FVector(
					Rand(-MaximumExtent.X, MaximumExtent.X),
					Rand(-MaximumExtent.Y, MaximumExtent.Y),
					Rand(-MaximumExtent.Z, MaximumExtent.Z));
			}
			else
			{
				const double VolumePick = Rand(0.f, 1.f) * TotalVolume;
				const int32 ChosenIndex = FMath::Min(Algo::LowerBound(CumulativeVolumes, VolumePick), ValidBoxes.Num() - 1);
				const FBox ChosenBox = ValidBoxes[ChosenIndex];
				Local = FVector(
					Rand(ChosenBox.Min.X, ChosenBox.Max.X),
					Rand(ChosenBox.Min.Y, ChosenBox.Max.Y),
					Rand(ChosenBox.Min.Z, ChosenBox.Max.Z));
			}
			return Params.Origin + (bHasRotation ? Params.Rotation.RotateVector(Local) : Local);
		});

		N_PICKER_ORIENTED_BOX_VLOG(!bSimpleMode)
	}
}

void FNOrientedBoxPicker::Random(TArray<FVector>& OutLocations, const FNOrientedBoxPickerParams& Params)
{
	N_PICKER_RANDOM_NONDETERMINISTIC
	GenerateOrientedBoxPoints(OutLocations, Params, [&Random](const float Min, const float Max) { return Random.FRandRange(Min, Max); });
}

void FNOrientedBoxPicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNOrientedBoxPickerParams& Params)
{
	const FRandomStream Random(Seed);
	GenerateOrientedBoxPoints(OutLocations, Params, [&Random](const float Min, const float Max) { return Random.FRandRange(Min, Max); });
	Seed = Random.GetCurrentSeed();
}

void FNOrientedBoxPicker::Next(TArray<FVector>& OutLocations, FNMersenneTwister& Random, const FNOrientedBoxPickerParams& Params)
{
	GenerateOrientedBoxPoints(OutLocations, Params, [&Random](const float Min, const float Max) { return Random.FloatRange(Min, Max); });
}
