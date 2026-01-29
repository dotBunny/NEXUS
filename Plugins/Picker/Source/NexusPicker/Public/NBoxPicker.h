// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NBoxPickerParams.h"
#include "NPickerMinimal.h"
#include "NPickerUtils.h"
#include "NRandom.h"

#if ENABLE_VISUAL_LOG
#define N_IMPLEMENT_VLOG_BOX_V2(HasMinimumDimensions) \
	if(Params.CachedWorld != nullptr && FVisualLogger::IsRecording()) \
	{ \
		if(HasMinimumDimensions) \
		{ \
			UE_VLOG_WIREBOX(Params.CachedWorld , LogNexusPicker, Verbose, Params.MinimumDimensions.MoveTo(Params.Origin), NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		} \
		if(Params.MaximumDimensions.IsValid != 0) \
		{ \
			UE_VLOG_WIREBOX(Params.CachedWorld , LogNexusPicker, Verbose, Params.MaximumDimensions.MoveTo(Params.Origin), NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		} \
		for (int i = 0; i < Params.Count; i++) \
		{ \
			UE_VLOG_LOCATION(Params.CachedWorld , LogNexusPicker, Verbose, OutLocations[OutLocationsStartIndex + i], NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocations[OutLocationsStartIndex + i].ToCompactString()); \
		} \
	}	
#else
#define N_IMPLEMENT_VLOG_BOX_V2(HasMinimumDimensions)
#endif




/**
 * Provides various functions for generating points inside or on the surface of the FBox using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/box/">FNBoxPicker</a>
 */
class NEXUSPICKER_API FNBoxPicker
{
public:
	
	/**
	 * Generates a deterministic point in relation to an FBox.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	 static void NextPoint(TArray<FVector>& OutLocations, const FNBoxPickerParams& Params)
	{
		const FVector MinimumExtent = 0.5f * (Params.MinimumDimensions.Max - Params.MinimumDimensions.Min);
		const FVector MaximumExtent = 0.5f * (Params.MaximumDimensions.Max - Params.MaximumDimensions.Min);
		
		// Only get this if we plan on iterating later
#if ENABLE_VISUAL_LOG			
		const int OutLocationsStartIndex = OutLocations.Num() - 1;
#endif
		
	 	
		const bool bSimpleMode = Params.MinimumDimensions.IsValid == 0;
		FVector OutLocation;
		
		if (Params.ProjectionMode == ENPickerProjectionMode::Projection && Params.CachedWorld != nullptr)
		{
			FHitResult HitResult(ForceInit);
			
			for (int i = 0; i < Params.Count; i++)
			{
				if (bSimpleMode)
				{
					OutLocation = Params.Origin + FVector(
					FNRandom::Deterministic.FloatRange(Params.MaximumDimensions.Min.X, Params.MaximumDimensions.Max.X),
					FNRandom::Deterministic.FloatRange(Params.MaximumDimensions.Min.Y, Params.MaximumDimensions.Max.Y),
					FNRandom::Deterministic.FloatRange(Params.MaximumDimensions.Min.Z, Params.MaximumDimensions.Max.Z));
				}
				else
				{
					OutLocation  = Params.Origin + FVector(
						FNRandom::Deterministic.Bool() ?
								FNRandom::Deterministic.FloatRange(-MinimumExtent.X, -MaximumExtent.X) :
								FNRandom::Deterministic.FloatRange(MinimumExtent.X, MaximumExtent.X),
						FNRandom::Deterministic.Bool() ?
								FNRandom::Deterministic.FloatRange(-MinimumExtent.Y, -MaximumExtent.Y) :
								FNRandom::Deterministic.FloatRange(MinimumExtent.Y, MaximumExtent.Y),
						FNRandom::Deterministic.Bool() ?
								FNRandom::Deterministic.FloatRange(-MinimumExtent.Z, -MaximumExtent.Z) :
								FNRandom::Deterministic.FloatRange(MinimumExtent.Z, MaximumExtent.Z));
				}
				N_IMPLEMENT_PICKER_PROJECTION_V2()
				
				OutLocations.Add(OutLocation);
			}
		}
		else if ( Params.ProjectionMode == ENPickerProjectionMode::NearestNavMesh)
		{
			// TODO: Implement nearest nav mesh projection
		}
		else
		{
			if (bSimpleMode)
			{
				OutLocations.Add(Params.Origin + FVector(
				FNRandom::Deterministic.FloatRange(Params.MaximumDimensions.Min.X, Params.MaximumDimensions.Max.X),
				FNRandom::Deterministic.FloatRange(Params.MaximumDimensions.Min.Y, Params.MaximumDimensions.Max.Y),
				FNRandom::Deterministic.FloatRange(Params.MaximumDimensions.Min.Z, Params.MaximumDimensions.Max.Z)));
			}
			else
			{
				OutLocations.Add(Params.Origin + FVector(
			FNRandom::Deterministic.Bool() ?
					FNRandom::Deterministic.FloatRange(-MinimumExtent.X, -MaximumExtent.X) :
					FNRandom::Deterministic.FloatRange(MinimumExtent.X, MaximumExtent.X),
			FNRandom::Deterministic.Bool() ?
					FNRandom::Deterministic.FloatRange(-MinimumExtent.Y, -MaximumExtent.Y) :
					FNRandom::Deterministic.FloatRange(MinimumExtent.Y, MaximumExtent.Y),
			FNRandom::Deterministic.Bool() ?
					FNRandom::Deterministic.FloatRange(-MinimumExtent.Z, -MaximumExtent.Z) :
					FNRandom::Deterministic.FloatRange(MinimumExtent.Z, MaximumExtent.Z)));
			}
		}
		
		N_IMPLEMENT_VLOG_BOX_V2(bSimpleMode)
	}
	/**
	 * Generates a random point in relation to an FBox.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	static void RandomPoint(TArray<FVector>& OutLocations, const FNBoxPickerParams& Params)
	{
		const FVector MinimumExtent = 0.5f * (Params.MinimumDimensions.Max - Params.MinimumDimensions.Min);
		const FVector MaximumExtent = 0.5f * (Params.MaximumDimensions.Max - Params.MaximumDimensions.Min);
	
#if ENABLE_VISUAL_LOG			
		const int OutLocationsStartIndex = OutLocations.Num() - 1;
#endif
		const bool bSimpleMode = Params.MinimumDimensions.IsValid == 0;
		
		if (Params.ProjectionMode == ENPickerProjectionMode::Projection && Params.CachedWorld != nullptr)
		{
			FVector OutLocation;
			FHitResult HitResult(ForceInit);
			for (int i = 0; i < Params.Count; i++)
			{
				if (bSimpleMode)
				{
					OutLocation = Params.Origin + FVector(FNRandom::NonDeterministic.FRandRange(Params.MaximumDimensions.Min.X, Params.MaximumDimensions.Max.X),
			FNRandom::NonDeterministic.FRandRange(Params.MaximumDimensions.Min.Y, Params.MaximumDimensions.Max.Y),
			FNRandom::NonDeterministic.FRandRange(Params.MaximumDimensions.Min.Z, Params.MaximumDimensions.Max.Z));
				}
				else
				{
					OutLocation = Params.Origin + FVector(
			FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f ?
					FNRandom::NonDeterministic.FRandRange(-MinimumExtent.X, -MaximumExtent.X) :
					FNRandom::NonDeterministic.FRandRange(MinimumExtent.X, MaximumExtent.X),
			FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f ?
					FNRandom::NonDeterministic.FRandRange(-MinimumExtent.Y, -MaximumExtent.Y) :
					FNRandom::NonDeterministic.FRandRange(MinimumExtent.Y, MaximumExtent.Y),
			FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f ?
					FNRandom::NonDeterministic.FRandRange(-MinimumExtent.Z, -MaximumExtent.Z) :
					FNRandom::NonDeterministic.FRandRange(MinimumExtent.Z, MaximumExtent.Z));
				}
				
				N_IMPLEMENT_PICKER_PROJECTION_V2()
				
				OutLocations.Add(OutLocation);
			}
		}
		else if ( Params.ProjectionMode == ENPickerProjectionMode::NearestNavMesh)
		{
			// TODO: Implement nearest nav mesh projection
		}
		else
		{
			for (int i = 0; i < Params.Count; i++)
			{
				if (bSimpleMode)
				{
					OutLocations.Add(Params.Origin + FVector(FNRandom::NonDeterministic.FRandRange(Params.MaximumDimensions.Min.X, Params.MaximumDimensions.Max.X),
			FNRandom::NonDeterministic.FRandRange(Params.MaximumDimensions.Min.Y, Params.MaximumDimensions.Max.Y),
			FNRandom::NonDeterministic.FRandRange(Params.MaximumDimensions.Min.Z, Params.MaximumDimensions.Max.Z)));
				}
				else
				{
					
					OutLocations.Add(Params.Origin + FVector(
			FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f ?
					FNRandom::NonDeterministic.FRandRange(-MinimumExtent.X, -MaximumExtent.X) :
					FNRandom::NonDeterministic.FRandRange(MinimumExtent.X, MaximumExtent.X),
			FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f ?
					FNRandom::NonDeterministic.FRandRange(-MinimumExtent.Y, -MaximumExtent.Y) :
					FNRandom::NonDeterministic.FRandRange(MinimumExtent.Y, MaximumExtent.Y),
			FNRandom::NonDeterministic.FRandRange(0.0f, 1.0f) >= 0.5f ?
					FNRandom::NonDeterministic.FRandRange(-MinimumExtent.Z, -MaximumExtent.Z) :
					FNRandom::NonDeterministic.FRandRange(MinimumExtent.Z, MaximumExtent.Z)));
				}
			}
		}
		N_IMPLEMENT_VLOG_BOX_V2(bSimpleMode)
		

	}
	
	
	/// ---- rebuilt stop
	
	FORCEINLINE static void NextPointInsideOrOn(FVector& OutLocation, const FVector& Origin, const FBox& MinimumDimensions, const FBox& MaximumDimensions)
	{
		TArray<FVector> OutLocations;
		
		FNBoxPickerParams Params;
		Params.Origin = Origin;
		Params.MinimumDimensions = MinimumDimensions;
		Params.MaximumDimensions = MaximumDimensions;
		
		NextPoint(OutLocations, Params);
		OutLocation = OutLocations[0];
	}
	FORCEINLINE static void NextPointInsideOrOnProjected(FVector& OutLocation, const FVector& Origin, const FBox& MinimumDimensions, const FBox& MaximumDimensions, N_VARIABLES_PICKER_PROJECTION())
	{
		TArray<FVector> OutLocations;
		
		FNBoxPickerParams Params;
		Params.Origin = Origin;
		Params.MinimumDimensions = MinimumDimensions;
		Params.MaximumDimensions = MaximumDimensions;
		
		Params.ProjectionMode = ENPickerProjectionMode::Projection;
		Params.Projection = Projection;
		Params.CollisionChannel = CollisionChannel;
		Params.CachedWorld = const_cast<UWorld*>(InWorld);
		
		NextPoint(OutLocations, Params);
		OutLocation = OutLocations[0];
	}
	FORCEINLINE static void NextPointInsideOrOnSimple(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
		TArray<FVector> OutLocations;
		
		FNBoxPickerParams Params;
		Params.Origin = Origin;
		Params.MaximumDimensions = Dimensions;
		
		NextPoint(OutLocations, Params);
		OutLocation = OutLocations[0];
	}
	FORCEINLINE static void NextPointInsideOrOnSimpleProjected(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions, N_VARIABLES_PICKER_PROJECTION())
	{
		TArray<FVector> OutLocations;
		
		FNBoxPickerParams Params;
		Params.Origin = Origin;
		Params.MaximumDimensions = Dimensions;
		
		Params.ProjectionMode = ENPickerProjectionMode::Projection;
		Params.Projection = Projection;
		Params.CollisionChannel = CollisionChannel;
		Params.CachedWorld = const_cast<UWorld*>(InWorld);
		
		NextPoint(OutLocations, Params);
		OutLocation = OutLocations[0];
	}
	FORCEINLINE static void RandomPointInsideOrOn(FVector& OutLocation, const FVector& Origin, const FBox& MinimumDimensions, const FBox& MaximumDimensions)
	{
	 	TArray<FVector> OutLocations;
		
	 	FNBoxPickerParams Params;
	 	Params.Origin = Origin;
	 	Params.MinimumDimensions = MinimumDimensions;
	 	Params.MaximumDimensions = MaximumDimensions;
		
	 	RandomPoint(OutLocations, Params);
	 	OutLocation = OutLocations[0];
	}
	FORCEINLINE static void RandomPointInsideOrOnProjected(FVector& OutLocation, const FVector& Origin, const FBox& MinimumDimensions, const FBox& MaximumDimensions, N_VARIABLES_PICKER_PROJECTION())
	{
	 	TArray<FVector> OutLocations;
		
	 	FNBoxPickerParams Params;
	 	Params.Origin = Origin;
	 	Params.MinimumDimensions = MinimumDimensions;
	 	Params.MaximumDimensions = MaximumDimensions;
		
	 	Params.ProjectionMode = ENPickerProjectionMode::Projection;
	 	Params.Projection = Projection;
	 	Params.CollisionChannel = CollisionChannel;
	 	Params.CachedWorld = const_cast<UWorld*>(InWorld);
		
	 	RandomPoint(OutLocations, Params);
	 	OutLocation = OutLocations[0];
	}
	FORCEINLINE static void RandomPointInsideOrOnSimple(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions)
	{
	 	TArray<FVector> OutLocations;
		
	 	FNBoxPickerParams Params;
	 	Params.Origin = Origin;
	 	Params.MaximumDimensions = Dimensions;
		
	 	RandomPoint(OutLocations, Params);
	 	OutLocation = OutLocations[0];
	}
	FORCEINLINE static void RandomPointInsideOrOnSimpleProjected(FVector& OutLocation, const FVector& Origin, const FBox& Dimensions, N_VARIABLES_PICKER_PROJECTION())
	{
	 	TArray<FVector> OutLocations;
		
	 	FNBoxPickerParams Params;
	 	Params.Origin = Origin;
	 	Params.MaximumDimensions = Dimensions;
		
	 	Params.ProjectionMode = ENPickerProjectionMode::Projection;
	 	Params.Projection = Projection;
	 	Params.CollisionChannel = CollisionChannel;
	 	Params.CachedWorld = const_cast<UWorld*>(InWorld);
		
	 	RandomPoint(OutLocations, Params);
	 	OutLocation = OutLocations[0];
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