// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSpherePicker.h"
#include "NPickerMinimal.h"
#include "NPickerProjection.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_SPHERE_PREFIX \
	N_PICKER_PARAMS_WORLD_SAFETY \
	const int32 OutLocationsStartIndex = OutLocations.Num(); \
	const float MinRadiusCubed = Params.MinimumRadius * Params.MinimumRadius * Params.MinimumRadius; \
	const float MaxRadiusCubed = Params.MaximumRadius * Params.MaximumRadius * Params.MaximumRadius; \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count); \
	if (Params.MinimumRadius < 0.f || Params.MaximumRadius < Params.MinimumRadius) \
	{ \
		UE_LOG(LogNexusPicker, Warning, TEXT("FNSpherePickerParams has invalid radius range (Min=%.2f, Max=%.2f); points may not be where you expect."), Params.MinimumRadius, Params.MaximumRadius); \
	}
// Inverse-CDF transform: uniform u in [0,1] maps to volume-weighted radius across the shell.
#define N_PICKER_SPHERE_POINT(FloatSingle, VectorRand) \
	[&]() -> FVector \
	{ \
		const float PointRadius = FMath::Pow(Random.FloatSingle() * (MaxRadiusCubed - MinRadiusCubed) + MinRadiusCubed, 1.0f / 3.0f); \
		return Params.Origin + (Random.VectorRand() * PointRadius); \
	}

#if ENABLE_VISUAL_LOG
#define N_PICKER_SPHERE_VLOG \
	if(CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		UE_VLOG_WIRESPHERE(CachedWorld, LogNexusPicker, Verbose, Params.Origin, Params.MinimumRadius, NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		UE_VLOG_WIRESPHERE(CachedWorld, LogNexusPicker, Verbose, Params.Origin, Params.MaximumRadius, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		for (int32 i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(CachedWorld , LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}
#else // !ENABLE_VISUAL_LOG
#define N_PICKER_SPHERE_VLOG
#endif // ENABLE_VISUAL_LOG

void FNSpherePicker::Random(TArray<FVector>& OutLocations, const FNSpherePickerParams& Params)
{
	N_PICKER_RANDOM_NONDETERMINISTIC
	N_PICKER_SPHERE_PREFIX
	FNPickerProjection::Emit(OutLocations, CachedWorld, Params, N_PICKER_SPHERE_POINT(FRand, VRand));
	N_PICKER_SPHERE_VLOG
}

void FNSpherePicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNSpherePickerParams& Params)
{
	const FRandomStream Random(Seed);
	N_PICKER_SPHERE_PREFIX
	FNPickerProjection::Emit(OutLocations, CachedWorld, Params, N_PICKER_SPHERE_POINT(FRand, VRand));
	N_PICKER_SPHERE_VLOG
	Seed = Random.GetCurrentSeed();
}

void FNSpherePicker::Next(TArray<FVector>& OutLocations, FNMersenneTwister& Random, const FNSpherePickerParams& Params)
{
	N_PICKER_SPHERE_PREFIX
	FNPickerProjection::Emit(OutLocations, CachedWorld, Params, N_PICKER_SPHERE_POINT(Float, VRand));
	N_PICKER_SPHERE_VLOG
}
