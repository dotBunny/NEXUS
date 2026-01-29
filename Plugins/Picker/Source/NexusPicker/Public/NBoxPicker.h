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
	
	/**
	 * Generates a deterministic point in relation to an FBox.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	static void Next(TArray<FVector>& OutLocations, const FNBoxPickerParams& Params)
	{
		const FVector MinimumExtent = 0.5f * (Params.MinimumDimensions.Max - Params.MinimumDimensions.Min);
		const FVector MaximumExtent = 0.5f * (Params.MaximumDimensions.Max - Params.MaximumDimensions.Min);
		
		// Only get this if we plan on iterating later
#if ENABLE_VISUAL_LOG			
		const int OutLocationsStartIndex = OutLocations.Num();
#endif
		
	 	
		const bool bSimpleMode = Params.MinimumDimensions.IsValid == 0;
		FVector OutLocation;
		
		if (Params.ProjectionMode == ENPickerProjectionMode::Projected && Params.CachedWorld != nullptr)
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
			for (int i = 0; i < Params.Count; i++)
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
		}
		
		N_IMPLEMENT_VLOG_BOX_V2(bSimpleMode)
	}

	/**
	 * Generates a random point in relation to an FBox.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param OutLocations An array to store the generated points.
	 * @param Params The parameters for the point generation.
	 */
	static void Random(TArray<FVector>& OutLocations, const FNBoxPickerParams& Params)
	{
		const FVector MinimumExtent = 0.5f * (Params.MinimumDimensions.Max - Params.MinimumDimensions.Min);
		const FVector MaximumExtent = 0.5f * (Params.MaximumDimensions.Max - Params.MaximumDimensions.Min);
	
#if ENABLE_VISUAL_LOG			
		const int OutLocationsStartIndex = OutLocations.Num();
#endif
		const bool bSimpleMode = Params.MinimumDimensions.IsValid == 0;
		
		if (Params.ProjectionMode == ENPickerProjectionMode::Projected && Params.CachedWorld != nullptr)
		{
			FVector OutLocation;
			FHitResult HitResult(ForceInit);
			for (int i = 0; i < Params.Count; i++)
			{
				if (bSimpleMode)
				{
					OutLocation = Params.Origin + FVector(
						FNRandom::NonDeterministic.FRandRange(Params.MaximumDimensions.Min.X, Params.MaximumDimensions.Max.X),
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
					OutLocations.Add(Params.Origin + FVector(
						FNRandom::NonDeterministic.FRandRange(Params.MaximumDimensions.Min.X, Params.MaximumDimensions.Max.X),
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

	/**
	 * Generates a random point in relation to an FBox.
	 * Useful for one-time random point generation with reproducible results.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to use.
	 * @param Params The parameters for the point generation.
	 */
	FORCEINLINE static void OneShot(TArray<FVector>& OutLocations, const int32 Seed, const FNBoxPickerParams& Params)
	{
		int32 DuplicateSeed = Seed;
	 	Tracked(OutLocations, DuplicateSeed, Params);
	}
	
	/**
	 * Generates a random point in relation to an FBox.
	 * Updates the seed value to enable sequential random point generation.
	 * @param OutLocations An array to store the generated points.
	 * @param Seed The random seed to start with, and update.
	 * @param Params The parameters for the point generation.
	 */
	static void Tracked(TArray<FVector>& OutLocations, int32& Seed, const FNBoxPickerParams& Params)
	{
	 	const FRandomStream RandomStream(Seed);
	 	const FVector MinimumExtent = 0.5f * (Params.MinimumDimensions.Max - Params.MinimumDimensions.Min);
	 	const FVector MaximumExtent = 0.5f * (Params.MaximumDimensions.Max - Params.MaximumDimensions.Min);
		
	 	// Only get this if we plan on iterating later
#if ENABLE_VISUAL_LOG			
	 	const int OutLocationsStartIndex = OutLocations.Num();
#endif
	 	
	 	const bool bSimpleMode = Params.MinimumDimensions.IsValid == 0;
	 	FVector OutLocation;	 
	 	
	 	if (Params.ProjectionMode == ENPickerProjectionMode::Projected && Params.CachedWorld != nullptr)
	 	{
	 		FHitResult HitResult(ForceInit);
	
	 		for (int i = 0; i < Params.Count; i++)
	 		{
	 			if (bSimpleMode)
	 			{
	 				OutLocation = Params.Origin + FVector(
	 					RandomStream.FRandRange(Params.MaximumDimensions.Min.X, Params.MaximumDimensions.Max.X),
	 					RandomStream.FRandRange(Params.MaximumDimensions.Min.Y, Params.MaximumDimensions.Max.Y),
	 					RandomStream.FRandRange(Params.MaximumDimensions.Min.Z, Params.MaximumDimensions.Max.Z));
	 			}
	 			else
	 			{
	 				OutLocation = Params.Origin + FVector(
	 					RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
	 					RandomStream.FRandRange(-MinimumExtent.X, -MaximumExtent.X) :
	 					RandomStream.FRandRange(MinimumExtent.X, MaximumExtent.X),
	 					RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
	 					RandomStream.FRandRange(-MinimumExtent.Y, -MaximumExtent.Y) :
	 					RandomStream.FRandRange(MinimumExtent.Y, MaximumExtent.Y),
	 					RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
	 					RandomStream.FRandRange(-MinimumExtent.Z, -MaximumExtent.Z) :
	 					RandomStream.FRandRange(MinimumExtent.Z, MaximumExtent.Z));
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
	 				OutLocations.Add(Params.Origin + FVector(
	 					RandomStream.FRandRange(Params.MaximumDimensions.Min.X, Params.MaximumDimensions.Max.X),
	 					RandomStream.FRandRange(Params.MaximumDimensions.Min.Y, Params.MaximumDimensions.Max.Y),
	 					RandomStream.FRandRange(Params.MaximumDimensions.Min.Z, Params.MaximumDimensions.Max.Z)));
	 			}
	 			else
	 			{
	 			
	 				OutLocations.Add(Params.Origin + FVector(
	 					RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
	 					RandomStream.FRandRange(-MinimumExtent.X, -MaximumExtent.X) :
	 					RandomStream.FRandRange(MinimumExtent.X, MaximumExtent.X),
	 					RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
	 					RandomStream.FRandRange(-MinimumExtent.Y, -MaximumExtent.Y) :
	 					RandomStream.FRandRange(MinimumExtent.Y, MaximumExtent.Y),
	 					RandomStream.FRandRange(0.0f, 1.0f) >= 0.5f ?
	 					RandomStream.FRandRange(-MinimumExtent.Z, -MaximumExtent.Z) :
	 					RandomStream.FRandRange(MinimumExtent.Z, MaximumExtent.Z)));
	 			}
	 		}
	 	}

	 	N_IMPLEMENT_VLOG_BOX_V2(bSimpleMode)
		
		Seed = RandomStream.GetCurrentSeed();
	}
	
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
	
	FORCEINLINE static TArray<bool> IsPointsInsideOrOn(const TArray<FVector>& Points, const FVector& Origin, const FBox& MinimumDimensions, const FBox& MaximumDimensions)
	{
		TArray<bool> OutResults;
		for (const FVector& Point : Points)
		{
			if (IsPointInsideOrOn(Origin, MinimumDimensions, Point) || !IsPointInsideOrOn(Origin, MaximumDimensions, Point))
			{
				OutResults.Add(false);
			}
			else
			{
				OutResults.Add(true);
			}
		}
		return MoveTemp(OutResults);
		
	}
};