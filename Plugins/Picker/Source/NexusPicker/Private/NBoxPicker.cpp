// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NBoxPicker.h"

#include "Algo/BinarySearch.h"
#include "NPickerMinimal.h"
#include "NPickerProjection.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_BOX_PREFIX \
	N_PICKER_PARAMS_WORLD_SAFETY \
	const int32 OutLocationsStartIndex = OutLocations.Num(); \
	const bool bSimpleMode = Params.MinimumBox.IsValid == 0; \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count); \
	if (bSimpleMode && Params.MaximumBox.IsValid == 0) \
	{ \
		for (int32 i = 0; i < Params.Count; i++) \
		{ \
			OutLocations.Add(Params.Origin); \
		} \
		UE_LOG(LogNexusPicker, Error, TEXT("Unable to pick points as FNBoxPickerParams has neither MinimumBox nor MaximumBox set. Defaulting to origin points.")) \
		return; \
	}

#if ENABLE_VISUAL_LOG
#define N_PICKER_BOX_VLOG_BOXES(Boxes) \
	if(CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		for(int32 v = 0; v < Boxes.Num(); v++) \
		{ \
			UE_VLOG_WIREBOX(CachedWorld , LogNexusPicker, Verbose, Boxes[v].ShiftBy(Params.Origin), NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		} \
	}
#define N_PICKER_BOX_VLOG(HasMinimumBox) \
	if(CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		if(HasMinimumBox) \
		{ \
			UE_VLOG_WIREBOX(CachedWorld , LogNexusPicker, Verbose, Params.MinimumBox.ShiftBy(Params.Origin), NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		} \
		if(Params.MaximumBox.IsValid != 0) \
		{ \
			UE_VLOG_WIREBOX(CachedWorld , LogNexusPicker, Verbose, Params.MaximumBox.ShiftBy(Params.Origin), NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		} \
		for (int32 i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(CachedWorld , LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else // !ENABLE_VISUAL_LOG
#define N_PICKER_BOX_VLOG_BOXES(Boxes)
#define N_PICKER_BOX_VLOG(HasMinimumBox)
#endif // ENABLE_VISUAL_LOG

namespace
{
	// Single source of truth for box point generation. Random/Tracked/Next differ only in how a float in
	// [Min,Max] is drawn, so that is the one parameter: Rand(Min, Max) -> float.
	template <typename FRandFloat>
	void GenerateBoxPoints(TArray<FVector>& OutLocations, const FNBoxPickerParams& Params, FRandFloat&& Rand)
	{
		N_PICKER_BOX_PREFIX

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
			N_PICKER_BOX_VLOG_BOXES(ValidBoxes)
		}

		FNPickerProjection::Emit(OutLocations, CachedWorld, Params, [&]() -> FVector
		{
			if (bSimpleMode)
			{
				return Params.Origin + FVector(
					Rand(Params.MaximumBox.Min.X, Params.MaximumBox.Max.X),
					Rand(Params.MaximumBox.Min.Y, Params.MaximumBox.Max.Y),
					Rand(Params.MaximumBox.Min.Z, Params.MaximumBox.Max.Z));
			}
			const double VolumePick = Rand(0.f, 1.f) * TotalVolume;
			const int32 ChosenIndex = FMath::Min(Algo::LowerBound(CumulativeVolumes, VolumePick), ValidBoxes.Num() - 1);
			const FBox ChosenBox = ValidBoxes[ChosenIndex];
			return Params.Origin + FVector(
				Rand(ChosenBox.Min.X, ChosenBox.Max.X),
				Rand(ChosenBox.Min.Y, ChosenBox.Max.Y),
				Rand(ChosenBox.Min.Z, ChosenBox.Max.Z));
		});

		N_PICKER_BOX_VLOG(!bSimpleMode)
	}
}

void FNBoxPicker::Random(TArray<FVector>& OutLocations, const FNBoxPickerParams& Params)
{
	N_PICKER_RANDOM_NONDETERMINISTIC
	GenerateBoxPoints(OutLocations, Params, [&Random](const float Min, const float Max) { return Random.FRandRange(Min, Max); });
}

void FNBoxPicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNBoxPickerParams& Params)
{
	const FRandomStream Random(Seed);
	GenerateBoxPoints(OutLocations, Params, [&Random](const float Min, const float Max) { return Random.FRandRange(Min, Max); });
	Seed = Random.GetCurrentSeed();
}

void FNBoxPicker::Next(TArray<FVector>& OutLocations, FNMersenneTwister& Random, const FNBoxPickerParams& Params)
{
	GenerateBoxPoints(OutLocations, Params, [&Random](const float Min, const float Max) { return Random.FloatRange(Min, Max); });
}
