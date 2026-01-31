// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NOrientedBoxPicker.h"
#include "NavigationSystem.h"
#include "NPickerMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#define N_PICKER_ORIENTED_BOX_PREFIX \
	const int OutLocationsStartIndex = OutLocations.Num(); \
	const bool bSimpleMode = Params.MinimumDimensions.IsZero(); \
	const bool bHasRotation = !Params.Rotation.IsZero(); \
	OutLocations.Reserve(OutLocationsStartIndex + Params.Count);
#define N_PICKER_ORIENTED_BOX_EXTENTS \
	const FVector MinimumExtent = 0.5f * Params.MinimumDimensions; \
	const FVector MaximumExtent = 0.5f * Params.MaximumDimensions;
#define N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE(FloatValue) \
	Params.Origin + FVector( \
		FloatValue(Params.MaximumDimensions.Min.X, Params.MaximumDimensions.Max.X), \
		FloatValue(Params.MaximumDimensions.Min.Y, Params.MaximumDimensions.Max.Y), \
		FloatValue(Params.MaximumDimensions.Min.Z, Params.MaximumDimensions.Max.Z))
#define N_PICKER_ORIENTED_BOX_LOCATION(FloatValue, BooleanValue) \
	Params.Origin + FVector( \
		BooleanValue ? FloatValue(-MinimumExtent.X, -MaximumExtent.X) : FloatValue(MinimumExtent.X, MaximumExtent.X), \
		BooleanValue ? FloatValue(-MinimumExtent.Y, -MaximumExtent.Y) : FloatValue(MinimumExtent.Y, MaximumExtent.Y), \
		BooleanValue ? FloatValue(-MinimumExtent.Z, -MaximumExtent.Z) : FloatValue(MinimumExtent.Z, MaximumExtent.Z))
#define N_PICKER_ORIENTED_BOX_LOCATION_SIMPLE_ROTATION(FloatValue) \
	Params.Origin + Params.Rotation.RotateVector(FVector( \
		FloatValue(Params.MaximumDimensions.Min.X, Params.MaximumDimensions.Max.X), \
		FloatValue(Params.MaximumDimensions.Min.Y, Params.MaximumDimensions.Max.Y), \
		FloatValue(Params.MaximumDimensions.Min.Z, Params.MaximumDimensions.Max.Z)))
#define N_PICKER_ORIENTED_BOX_LOCATION_ROTATION(FloatValue, BooleanValue) \
	Params.Origin + Params.Rotation.RotateVector(FVector( \
		BooleanValue ? FloatValue(-MinimumExtent.X, -MaximumExtent.X) : FloatValue(MinimumExtent.X, MaximumExtent.X), \
		BooleanValue ? FloatValue(-MinimumExtent.Y, -MaximumExtent.Y) : FloatValue(MinimumExtent.Y, MaximumExtent.Y), \
		BooleanValue ? FloatValue(-MinimumExtent.Z, -MaximumExtent.Z) : FloatValue(MinimumExtent.Z, MaximumExtent.Z)))

#define RANDOM_FLOAT_RANGE FNRandom::Deterministic.FloatRange
#define RANDOM_BOOL FNRandom::Deterministic.Bool()
void FNOrientedBoxPicker::Next(TArray<FVector>& OutLocations, const FNOrientedBoxPickerParams& Params)
{
	N_PICKER_ORIENTED_BOX_PREFIX

}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_BOOL

#define RANDOM_FLOAT_RANGE FNRandom::NonDeterministic.FRandRange
#define RANDOM_BOOL FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f
void FNOrientedBoxPicker::Random(TArray<FVector>& OutLocations, const FNOrientedBoxPickerParams& Params)
{
	N_PICKER_ORIENTED_BOX_PREFIX
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_BOOL

#define RANDOM_FLOAT_RANGE RandomStream.FRandRange
#define RANDOM_BOOL RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f
void FNOrientedBoxPicker::Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNOrientedBoxPickerParams& Params)
{
	const FRandomStream RandomStream(Seed);
	
	N_PICKER_ORIENTED_BOX_PREFIX
	Seed = RandomStream.GetCurrentSeed();
}
#undef RANDOM_FLOAT_RANGE
#undef RANDOM_BOOL
