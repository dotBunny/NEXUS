// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCirclePicker.h"
#include "NPickerMinimal.h"
#include "NPickerProjection.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_CIRCLE_PREFIX \
	N_PICKER_PARAMS_WORLD_SAFETY \
	const int32 OutLocationsStartIndex = OutLocations.Num(); \
	const bool bSimpleMode = Params.Rotation.IsZero(); \
	const float MinRadiusSq = Params.MinimumRadius * Params.MinimumRadius; \
	const float MaxRadiusSq = Params.MaximumRadius * Params.MaximumRadius; \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count); \
	if (Params.MinimumRadius < 0.f || Params.MaximumRadius < Params.MinimumRadius) \
	{ \
		UE_LOG(LogNexusPicker, Warning, TEXT("FNCirclePickerParams has invalid radius range (Min=%.2f, Max=%.2f); points may not be where you expect."), Params.MinimumRadius, Params.MaximumRadius); \
	}

#if ENABLE_VISUAL_LOG
#define N_PICKER_CIRCLE_VLOG(HasMinimumDimensions) \
	if(CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		if(HasMinimumDimensions) \
		{ \
			UE_VLOG_WIRECIRCLE(CachedWorld, LogNexusPicker, Verbose, Params.Origin, Params.Rotation.RotateVector(FVector::UpVector), Params.MinimumRadius, NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		} \
		UE_VLOG_WIRECIRCLE(CachedWorld, LogNexusPicker, Verbose, Params.Origin, Params.Rotation.RotateVector(FVector::UpVector), Params.MaximumRadius, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		for (int32 i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(CachedWorld, LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else // !ENABLE_VISUAL_LOG
#define N_PICKER_CIRCLE_VLOG(HasMinimumDimensions)
#endif // ENABLE_VISUAL_LOG

namespace
{
	// Single source of truth for circle point generation. Random/Tracked/Next differ only in how a uniform
	// float in [0,1) is drawn, so that is the one parameter: RandUnit() -> float.
	template <typename FRandUnit>
	void GenerateCirclePoints(TArray<FVector>& OutLocations, const FNCirclePickerParams& Params, FRandUnit&& RandUnit)
	{
		N_PICKER_CIRCLE_PREFIX
		FNPickerProjection::Emit(OutLocations, CachedWorld, Params, [&]() -> FVector
		{
			const float PointTheta = RandUnit() * 2.0f * PI;
			// Inverse-CDF transform: uniform u in [0,1] maps to area-weighted radius across the annulus.
			const float PointRadius = FMath::Sqrt(RandUnit() * (MaxRadiusSq - MinRadiusSq) + MinRadiusSq);
			const FVector Local((PointRadius * FMath::Cos(PointTheta)), (PointRadius * FMath::Sin(PointTheta)), 0.f);
			return Params.Origin + (bSimpleMode ? Local : Params.Rotation.RotateVector(Local));
		});
		N_PICKER_CIRCLE_VLOG(Params.MinimumRadius > 0.f)
	}
}

void FNCirclePicker::Random(TArray<FVector>& OutLocations, const FNCirclePickerParams& Params)
{
	N_PICKER_RANDOM_NONDETERMINISTIC
	GenerateCirclePoints(OutLocations, Params, [&Random] { return Random.FRand(); });
}

void FNCirclePicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNCirclePickerParams& Params)
{
	const FRandomStream Random(Seed);
	GenerateCirclePoints(OutLocations, Params, [&Random] { return Random.FRand(); });
	Seed = Random.GetCurrentSeed();
}

void FNCirclePicker::Next(TArray<FVector>& OutLocations, FNMersenneTwister& Random, const FNCirclePickerParams& Params)
{
	GenerateCirclePoints(OutLocations, Params, [&Random] { return Random.Float(); });
}
