// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NSplinePicker.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NSplinePickerLibrary.generated.h"

/**
 * Provides various functions for generating points along a USplineComponent spline using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/spline/">UNSplinePickerLibrary</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS: Spline Picker Library")
class NEXUSPICKER_API UNSplinePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Generates a deterministic point on a spline.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param Params The parameters for the point generation. 
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	 
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Next Point", Category = "NEXUS|Picker|Spline", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/spline/#next-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> NextPoint(UPARAM(ref) FNSplinePickerParams& Params, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNSplinePicker::Next(ReturnLocations, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point on a spline.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param Params The parameters for the point generation. 
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	 
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random Point", Category = "NEXUS|Picker|Spline", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/spline/#random-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> RandomPoint(UPARAM(ref) FNSplinePickerParams& Params, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNSplinePicker::Random(ReturnLocations, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point on a spline.
	 * Updates the seed value to enable sequential random point generation.	 
	 * @param Params The parameters for the point generation.
	 * @param Seed The random seed to start with, and update.	  
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Tracked Point", Category = "NEXUS|Picker|Spline", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/spline/#tracked-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> TrackedPoint(UPARAM(ref) FNSplinePickerParams& Params, UPARAM(ref) int32& Seed, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNSplinePicker::Tracked(ReturnLocations, Seed, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point on a spline.
	 * Useful for one-time random point generation with reproducible results. 
	 * @param Params The parameters for the point generation.
	 * @param Seed The random seed to start with, and update.	  
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: One-Shot Point", Category = "NEXUS|Picker|Spline", 
	meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/spline/#one-shot-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> OneShotPoint(UPARAM(ref) FNSplinePickerParams& Params, const int32 Seed, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNSplinePicker::OneShot(ReturnLocations, Seed, Params);
		return MoveTemp(ReturnLocations);
	}

	/**
	 * Checks if a point is on a spline within a specified tolerance.
	 * Uses the N_PICKER_TOLERANCE defined in NPickerUtils.h for proximity checking.
	 * @param SplineComponent The spline component to check against.
	 * @param Point The point to check.
	 * @return True if the point is on the spline within the tolerance, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Spline: Is Point On?", Category = "NEXUS|Picker|Spline")
	static bool IsPointOn(const USplineComponent* SplineComponent, const FVector& Point)
	{
		return FNSplinePicker::IsPointOn(SplineComponent, Point);
	}
	
	/**
	 * Checks if multiple points are on a spline within a specified tolerance.
	 * @param Points The array of points to check.
	 * @param SplineComponent The spline component to check against.
	 * @return An array of boolean values indicating if each point is on a spline within a specified tolerance.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Spline: Is Points On?", Category = "NEXUS|Picker|Spline")
	static TArray<bool> IsPointsOn(const TArray<FVector>& Points, const USplineComponent* SplineComponent)
	{
		return FNSplinePicker::IsPointsOn(Points, SplineComponent);
	}
};