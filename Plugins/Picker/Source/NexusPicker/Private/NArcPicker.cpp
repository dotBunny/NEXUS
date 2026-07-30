// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NArcPicker.h"

#include "NPickerMinimal.h"
#include "NPickerProjection.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_ARC_PREFIX \
	N_PICKER_PARAMS_WORLD_SAFETY \
	const int32 OutLocationsStartIndex = OutLocations.Num(); \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count); \
	const float MaxDegrees = (Params.Degrees * 0.5f); \
	const float MinDegrees = -MaxDegrees; \
	const float MinDistanceSq = Params.MinimumDistance * Params.MinimumDistance; \
	const float MaxDistanceSq = Params.MaximumDistance * Params.MaximumDistance; \
	if (Params.MinimumDistance < 0.f || Params.MaximumDistance < Params.MinimumDistance) \
	{ \
		UE_LOG(LogNexusPicker, Warning, TEXT("FNArcPickerParams has invalid distance range (Min=%.2f, Max=%.2f); points may not be where you expect."), Params.MinimumDistance, Params.MaximumDistance); \
	}

#if ENABLE_VISUAL_LOG
#define N_PICKER_RADIAL_VLOG \
	if(CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		const float MaxAngle = FMath::DegreesToRadians(MaxDegrees); \
		const float MinAngle = FMath::DegreesToRadians(MinDegrees); \
		const FVector InMinPoint = Params.Origin + Params.Rotation.RotateVector(FVector(FMath::Cos(MinAngle), FMath::Sin(MinAngle), 0) * Params.MinimumDistance); \
		const FVector OutMinPoint = Params.Origin + Params.Rotation.RotateVector(FVector(FMath::Cos(MinAngle), FMath::Sin(MinAngle), 0) * Params.MaximumDistance); \
		const FVector InMaxPoint = Params.Origin + Params.Rotation.RotateVector(FVector(FMath::Cos(MaxAngle), FMath::Sin(MaxAngle), 0) * Params.MinimumDistance); \
		const FVector OutMaxPoint = Params.Origin + Params.Rotation.RotateVector(FVector(FMath::Cos(MaxAngle), FMath::Sin(MaxAngle), 0) *Params.MaximumDistance); \
		UE_VLOG_SEGMENT(CachedWorld, LogNexusPicker, Verbose, InMinPoint, OutMinPoint, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		UE_VLOG_SEGMENT(CachedWorld, LogNexusPicker, Verbose, InMaxPoint, OutMaxPoint, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		for (int32 i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(CachedWorld , LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else // !ENABLE_VISUAL_LOG
#define N_PICKER_RADIAL_VLOG
#endif // ENABLE_VISUAL_LOG

// Inverse-CDF transform: uniform u in [0,1] maps to area-weighted radius across the annular sector.
#define N_PICKER_ARC_POINT(FloatRange) \
	[&]() -> FVector \
	{ \
		const float RandomAngle = FMath::DegreesToRadians(Random.FloatRange(MinDegrees, MaxDegrees)); \
		const float RandomDistance = FMath::Sqrt(Random.FloatRange(MinDistanceSq, MaxDistanceSq)); \
		return Params.Origin + Params.Rotation.RotateVector(FVector(FMath::Cos(RandomAngle), FMath::Sin(RandomAngle), 0) * RandomDistance); \
	}

void FNArcPicker::Random(TArray<FVector>& OutLocations, const FNArcPickerParams& Params)
{
	N_PICKER_RANDOM_NONDETERMINISTIC
	N_PICKER_ARC_PREFIX
	FNPickerProjection::Emit(OutLocations, CachedWorld, Params, N_PICKER_ARC_POINT(FRandRange));
	N_PICKER_RADIAL_VLOG
}

void FNArcPicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNArcPickerParams& Params)
{
	const FRandomStream Random(Seed);
	N_PICKER_ARC_PREFIX
	FNPickerProjection::Emit(OutLocations, CachedWorld, Params, N_PICKER_ARC_POINT(FRandRange));
	N_PICKER_RADIAL_VLOG
	Seed = Random.GetCurrentSeed();
}

void FNArcPicker::Next(TArray<FVector>& OutLocations, FNMersenneTwister& Random, const FNArcPickerParams& Params)
{
	N_PICKER_ARC_PREFIX
	FNPickerProjection::Emit(OutLocations, CachedWorld, Params, N_PICKER_ARC_POINT(FloatRange));
	N_PICKER_RADIAL_VLOG
}
