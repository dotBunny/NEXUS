// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NBoxPickerParams.h"
#include "NPickerMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

/**
 * Provides various functions for generating points inside or on the surface of the FBox using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/box/">FNBoxPicker</a>
 */
class NEXUSPICKER_API FNBoxPicker
{
public:

	// NEXT POINT
	
	FORCEINLINE static void NextPointInsideOrOn2(FVector& OutLocation, const FNBoxPickerParams& Params)
	{
		const FVector MinimumExtent = 0.5f * (Params.MinimumDimensions.Max - Params.MinimumDimensions.Min);
		const FVector MaximumExtent = 0.5f * (Params.MaximumDimensions.Max - Params.MaximumDimensions.Min);
		
		OutLocation = Params.Origin + FVector(
		FNRandom::Deterministic.Bool() ?
				FNRandom::Deterministic.FloatRange(-MinimumExtent.X, -MaximumExtent.X) :
				FNRandom::Deterministic.FloatRange(MinimumExtent.X, MaximumExtent.X),
		FNRandom::Deterministic.Bool() ?
				FNRandom::Deterministic.FloatRange(-MinimumExtent.Y, -MaximumExtent.Y) :
				FNRandom::Deterministic.FloatRange(MinimumExtent.Y, MaximumExtent.Y),
		FNRandom::Deterministic.Bool() ?
				FNRandom::Deterministic.FloatRange(-MinimumExtent.Z, -MaximumExtent.Z) :
				FNRandom::Deterministic.FloatRange(MinimumExtent.Z, MaximumExtent.Z));

//		N_IMPLEMENT_VLOG_BOX()
		if(Params.World != nullptr)
		{
#if ENABLE_VISUAL_LOG			
			if (FVisualLogger::IsRecording())
			{
				UE_VLOG_WIREBOX(Params.World , LogNexusPicker, Verbose, Params.MinimumDimensions.MoveTo(Params.Origin), NEXUS::Picker::VLog::InnerColor, TEXT(""));
				UE_VLOG_WIREBOX(Params.World , LogNexusPicker, Verbose, Params.MaximumDimensions.MoveTo(Params.Origin), NEXUS::Picker::VLog::OuterColor, TEXT(""));
				UE_VLOG_LOCATION(Params.World , LogNexusPicker, Verbose, OutLocation, NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocation.ToCompactString());
			}
#endif			
		}
	}
	
	/**
	 * Generates a deterministic point inside or on the surface of the FBox.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the FBox.
	 * @param MinimumDimensions The minimum dimensions to use when generating a point.
	 * @param MaximumDimensions The maximum dimensions to use when generating a point.
	 */
	FORCEINLINE static void NextPointInsideOrOn(FVector& OutLocation, const FVector& Origin, const FBox& MinimumDimensions, const FBox& MaximumDimensions)
	{
		const FVector MinimumExtent = 0.5f * (MinimumDimensions.Max - MinimumDimensions.Min);
		const FVector MaximumExtent = 0.5f * (MaximumDimensions.Max - MaximumDimensions.Min);
		
		OutLocation = Origin + FVector(
		FNRandom::Deterministic.Bool() ?
				FNRandom::Deterministic.FloatRange(-MinimumExtent.X, -MaximumExtent.X) :
				FNRandom::Deterministic.FloatRange(MinimumExtent.X, MaximumExtent.X),
		FNRandom::Deterministic.Bool() ?
				FNRandom::Deterministic.FloatRange(-MinimumExtent.Y, -MaximumExtent.Y) :
				FNRandom::Deterministic.FloatRange(MinimumExtent.Y, MaximumExtent.Y),
		FNRandom::Deterministic.Bool() ?
				FNRandom::Deterministic.FloatRange(-MinimumExtent.Z, -MaximumExtent.Z) :
				FNRandom::Deterministic.FloatRange(MinimumExtent.Z, MaximumExtent.Z));

		N_IMPLEMENT_VLOG_BOX()
	}

	/**
	 * Generates a deterministic point inside or on the surface of the FBox.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the FBox.
	 * @param MinimumDimensions The minimum dimensions to use when generating a point.
	 * @param MaximumDimensions The maximum dimensions to use when generating a point.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void NextPointInsideOrOnProjected(FVector& OutLocation, const FVector& Origin, const FBox& MinimumDimensions, const FBox& MaximumDimensions, N_VARIABLES_PICKER_PROJECTION())
	{
		NextPointInsideOrOn(OutLocation, Origin, MinimumDimensions, MaximumDimensions);
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_BOX_PROJECTION()
	}

	/**
	 * Generates a deterministic point inside or on the surface of the FBox.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 */
	FORCEINLINE static void NextPointInsideOrOnSimple(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
		OutLocation = Origin + FVector(
			FNRandom::Deterministic.FloatRange(Dimensions.Min.X, Dimensions.Max.X),
		FNRandom::Deterministic.FloatRange(Dimensions.Min.Y, Dimensions.Max.Y),
		FNRandom::Deterministic.FloatRange(Dimensions.Min.Z, Dimensions.Max.Z));

		N_IMPLEMENT_VLOG_BOX_SIMPLE()
	}

	/**
	 * Generates a deterministic point inside or on the surface of the FBox, then projects it to the world.
	 * @param OutLocation [out] The generated and grounded point location.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void NextPointInsideOrOnSimpleProjected(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions, N_VARIABLES_PICKER_PROJECTION())
	{
		NextPointInsideOrOnSimple(OutLocation, Origin, Dimensions);
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_BOX_SIMPLE_PROJECTION()
	}

	// RANDOM POINT

	/**
	 * Generates a random point inside or on the surface of the FBox.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the FBox.
	 * @param MinimumDimensions The minimum dimensions to use when generating a point.
	 * @param MaximumDimensions The maximum dimensions to use when generating a point.
	 */
	FORCEINLINE static void RandomPointInsideOrOn(FVector& OutLocation, const FVector& Origin, const FBox& MinimumDimensions, const FBox& MaximumDimensions)
	{
		const FVector MinimumExtent = 0.5f * (MinimumDimensions.Max - MinimumDimensions.Min);
		const FVector MaximumExtent = 0.5f * (MaximumDimensions.Max - MaximumDimensions.Min);
		
		OutLocation = Origin + FVector(
		FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f ?
				FNRandom::NonDeterministic.FRandRange(-MinimumExtent.X, -MaximumExtent.X) :
				FNRandom::NonDeterministic.FRandRange(MinimumExtent.X, MaximumExtent.X),
		FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f ?
				FNRandom::NonDeterministic.FRandRange(-MinimumExtent.Y, -MaximumExtent.Y) :
				FNRandom::NonDeterministic.FRandRange(MinimumExtent.Y, MaximumExtent.Y),
		FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f ?
				FNRandom::NonDeterministic.FRandRange(-MinimumExtent.Z, -MaximumExtent.Z) :
				FNRandom::NonDeterministic.FRandRange(MinimumExtent.Z, MaximumExtent.Z));

		N_IMPLEMENT_VLOG_BOX()
	}

	// #SONARQUBE-DISABLE-CPP_S107 Verbosity necessary.
	/**
	 * Generates a random point inside or on the surface of the FBox, then projects it to the world.
	 * @param OutLocation [out] The generated and grounded point location.
	 * @param Origin The center point of the FBox.
	 * @param MinimumDimensions The minimum dimensions to use when generating a point.
	 * @param MaximumDimensions The maximum dimensions to use when generating a point.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomPointInsideOrOnProjected(FVector& OutLocation, const FVector& Origin, const FBox& MinimumDimensions, const FBox& MaximumDimensions, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomPointInsideOrOn(OutLocation, Origin, MinimumDimensions, MaximumDimensions);
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_BOX_PROJECTION()
	}
	// #SONARQUBE-ENABLE

	/**
	 * Generates a random point inside or on the surface of the FBox.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 */
	FORCEINLINE static void RandomPointInsideOrOnSimple(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
		OutLocation = Origin + FVector(FNRandom::NonDeterministic.FRandRange(Dimensions.Min.X, Dimensions.Max.X),
			FNRandom::NonDeterministic.FRandRange(Dimensions.Min.Y, Dimensions.Max.Y),
			FNRandom::NonDeterministic.FRandRange(Dimensions.Min.Z, Dimensions.Max.Z));
		N_IMPLEMENT_VLOG_BOX_SIMPLE()
	}

	/**
	 * Generates a random point inside or on the surface of the FBox, then projects it to the world.
	 * @param OutLocation [out] The generated and grounded point location.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomPointInsideOrOnSimpleProjected(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions, N_VARIABLES_PICKER_PROJECTION())
	{
		RandomPointInsideOrOnSimple(OutLocation, Origin, Dimensions);
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_BOX_SIMPLE_PROJECTION()
	}

	// RANDOM ONE-SHOT POINT
	
	/**
	 * Generates a random point inside or on the surface of the FBox using a provided seed.
	 * Useful for one-time random point generation with reproducible results.
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the FBox.
	 * @param MinimumDimensions The minimum dimensions to use when generating a point.
	 * @param MaximumDimensions The maximum dimensions to use when generating a point.
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOn(const int32 Seed, FVector& OutLocation, const FVector& Origin, const FBox& MinimumDimensions, const FBox& MaximumDimensions)
	{
		const FRandomStream RandomStream(Seed);

		const FVector MinimumExtent = 0.5f * (MinimumDimensions.Max - MinimumDimensions.Min);
		const FVector MaximumExtent = 0.5f * (MaximumDimensions.Max - MaximumDimensions.Min);
		
		OutLocation = Origin + FVector(
		RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
				RandomStream.FRandRange(-MinimumExtent.X, -MaximumExtent.X) :
				RandomStream.FRandRange(MinimumExtent.X, MaximumExtent.X),
		RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
				RandomStream.FRandRange(-MinimumExtent.Y, -MaximumExtent.Y) :
				RandomStream.FRandRange(MinimumExtent.Y, MaximumExtent.Y),
		RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
				RandomStream.FRandRange(-MinimumExtent.Z, -MaximumExtent.Z) :
				RandomStream.FRandRange(MinimumExtent.Z, MaximumExtent.Z));

		N_IMPLEMENT_VLOG_BOX()
	}

	// #SONARQUBE-DISABLE-CPP_S107 Verbosity necessary.
	/**
	 * Generates a random point inside or on the surface of the FBox using a provided seed, then projects it to the world.	 
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated and grounded point location.
	 * @param Origin The center point of the FBox.
	 * @param MinimumDimensions The minimum dimensions to use when generating a point.
	 * @param MaximumDimensions The maximum dimensions to use when generating a point.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOnProjected(const int32 Seed, FVector& OutLocation, const FVector& Origin, const FBox& MinimumDimensions, const FBox& MaximumDimensions,
		N_VARIABLES_PICKER_PROJECTION())
	{
		RandomOneShotPointInsideOrOn(Seed, OutLocation, Origin, MinimumDimensions, MaximumDimensions);
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_BOX_PROJECTION()
	}
	// #SONARQUBE-ENABLE

	/**
	 * Generates a random point inside or on the surface of the FBox using a provided seed.
	 * Useful for one-time random point generation with reproducible results.
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOnSimple(const int32 Seed, FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = Origin + FVector(RandomStream.FRandRange(Dimensions.Min.X, Dimensions.Max.X),
				RandomStream.FRandRange(Dimensions.Min.Y, Dimensions.Max.Y),
				RandomStream.FRandRange(Dimensions.Min.Z, Dimensions.Max.Z));

		N_IMPLEMENT_VLOG_BOX_SIMPLE()
	}

	/**
	 * Generates a random point inside or on the surface of the FBox using a provided seed, then projects it to the world.	 
	 * @param Seed The random seed to use.
	 * @param OutLocation [out] The generated and grounded point location.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomOneShotPointInsideOrOnSimpleProjected(const int32 Seed, FVector& OutLocation, const FVector& Origin, const FBox& Dimensions,
		N_VARIABLES_PICKER_PROJECTION())
	{
		RandomOneShotPointInsideOrOnSimple(Seed, OutLocation, Origin, Dimensions);
		
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_BOX_SIMPLE_PROJECTION()
	}

	// RANDOM TRACKED POINT

	/**
	 * Generates a random point inside or on the surface of the FBox while tracking the random seed state.
	 * Updates the seed value to enable sequential random point generation.
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the FBox.
	 * @param MinimumDimensions The minimum dimensions to use when generating a point.
	 * @param MaximumDimensions The maximum dimensions to use when generating a point.
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOn(int32& Seed, FVector& OutLocation, const FVector& Origin, const FBox& MinimumDimensions, const FBox& MaximumDimensions)
	{
		const FRandomStream RandomStream(Seed);

		const FVector MinimumExtent = 0.5f * (MinimumDimensions.Max - MinimumDimensions.Min);
		const FVector MaximumExtent = 0.5f * (MaximumDimensions.Max - MaximumDimensions.Min);
		
		OutLocation = Origin + FVector(
		RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
				RandomStream.FRandRange(-MinimumExtent.X, -MaximumExtent.X) :
				RandomStream.FRandRange(MinimumExtent.X, MaximumExtent.X),
		RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
				RandomStream.FRandRange(-MinimumExtent.Y, -MaximumExtent.Y) :
				RandomStream.FRandRange(MinimumExtent.Y, MaximumExtent.Y),
		RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
				RandomStream.FRandRange(-MinimumExtent.Z, -MaximumExtent.Z) :
				RandomStream.FRandRange(MinimumExtent.Z, MaximumExtent.Z));
		
		Seed = RandomStream.GetCurrentSeed();
		
		N_IMPLEMENT_VLOG_BOX()
	}

	// #SONARQUBE-DISABLE-CPP_S107 Verbosity necessary.
	/**
	 * Generates a random point inside or on the surface of the FBox while tracking the random seed state, then grounds it to the world.
	 * Updates the seed value to enable sequential random point generation.	
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated and grounded point location.
	 * @param Origin The center point of the FBox.
	 * @param MinimumDimensions The minimum dimensions to use when generating a point.
	 * @param MaximumDimensions The maximum dimensions to use when generating a point.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOnProjected(int32& Seed, FVector& OutLocation, const FVector& Origin, const FBox& MinimumDimensions, const FBox& MaximumDimensions,
		N_VARIABLES_PICKER_PROJECTION())
	{
		RandomTrackedPointInsideOrOn(Seed, OutLocation, Origin, MinimumDimensions, MaximumDimensions);
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_BOX_PROJECTION()
	}
	// #SONARQUBE-ENABLE

	/**
	 * Generates a random point inside or on the surface of the FBox while tracking the random seed state.
	 * Updates the seed value to enable sequential random point generation.
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated point location.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOnSimple(int32& Seed, FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
		const FRandomStream RandomStream(Seed);
		OutLocation = Origin + FVector(RandomStream.FRandRange(Dimensions.Min.X, Dimensions.Max.X),
				RandomStream.FRandRange(Dimensions.Min.Y, Dimensions.Max.Y),
				RandomStream.FRandRange(Dimensions.Min.Z, Dimensions.Max.Z));
		Seed = RandomStream.GetCurrentSeed();

		N_IMPLEMENT_VLOG_BOX_SIMPLE()
	}

	/**
	 * Generates a random point inside or on the surface of the FBox while tracking the random seed state, then grounds it to the world.
	 * Updates the seed value to enable sequential random point generation.	
	 * @param Seed [in,out] The random seed to use and update.
	 * @param OutLocation [out] The generated and grounded point location.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @param InWorld The world context for line tracing.
	 * @param Projection Direction and distance for the line trace.
	 * @param CollisionChannel The collision channel to use for tracing.
	 */
	FORCEINLINE static void RandomTrackedPointInsideOrOnSimpleProjected(int32& Seed, FVector& OutLocation, const FVector& Origin, const FBox& Dimensions,
		N_VARIABLES_PICKER_PROJECTION())
	{
		RandomTrackedPointInsideOrOnSimple(Seed, OutLocation, Origin, Dimensions);
		N_IMPLEMENT_PICKER_PROJECTION()
		N_IMPLEMENT_VLOG_BOX_SIMPLE_PROJECTION()
	}

	// ASSERT

	/**
	 * Checks if a point is inside or on the surface of the FBox.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the surface of the FBox, false otherwise.
	 */
	FORCEINLINE static bool IsPointInsideOrOn(const FVector& Origin, const FBox& Dimensions, const FVector& Point)
	{
		const FBox Box(Origin + Dimensions.Min, Origin + Dimensions.Max);
		return Box.IsInsideOrOn(Point);
	}
};