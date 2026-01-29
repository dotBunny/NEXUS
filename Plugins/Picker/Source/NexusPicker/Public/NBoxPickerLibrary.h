// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NBoxPicker.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NBoxPickerLibrary.generated.h"

/**
 * Provides various functions for generating points inside or on the surface of the FBox using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/box/">UNBoxPickerLibrary</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS: Box Picker Library")
class NEXUSPICKER_API UNBoxPickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	/**
	 * Generates a deterministic point in relation to an FBox.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param Params The parameters for the point generation. 
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	 
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Box: Next Point", Category = "NEXUS|Picker|Box", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/box/#next-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> NextPoint(UPARAM(ref) FNBoxPickerParams& Params, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNBoxPicker::Next(ReturnLocations, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point in relation to an FBox.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param Params The parameters for the point generation. 
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	 
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random Point", Category = "NEXUS|Picker|Box", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/box/#random-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> RandomPoint(UPARAM(ref) FNBoxPickerParams& Params, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNBoxPicker::Random(ReturnLocations, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a random point in relation to an FBox.
	 * Updates the seed value to enable sequential random point generation.	 
	 * @param Params The parameters for the point generation.
	 * @param Seed The random seed to start with, and update.	  
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Box: Tracked Point", Category = "NEXUS|Picker|Box", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/box/#tracked-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> TrackedPoint(UPARAM(ref) FNBoxPickerParams& Params, UPARAM(ref) int32& Seed, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNBoxPicker::Tracked(ReturnLocations, Seed, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a random point in relation to an FBox.
	 * Useful for one-time random point generation with reproducible results. 
	 * @param Params The parameters for the point generation.
	 * @param Seed The random seed to start with, and update.	  
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "Box: One-Shot Point", Category = "NEXUS|Picker|Box", 
	meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/box/#one-shot-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> OneShotPoint(UPARAM(ref) FNBoxPickerParams& Params, const int32 Seed, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNBoxPicker::OneShot(ReturnLocations, Seed, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Checks if a point is inside or on the FBox.
	 * @param Origin      The center point of the FBox.
	 * @param Dimensions  The dimensions of the FBox.
	 * @param Point       The point to check.
	 * @return            True if the point is inside or on the FBox, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Box: Is Point Inside Or On?", Category = "NEXUS|Picker|Box")
	static bool IsPointInsideOrOn(const FVector& Origin, const FBox& Dimensions, const FVector& Point)
	{
		return FNBoxPicker::IsPointInsideOrOn(Origin, Dimensions, Point);
	}
	
	UFUNCTION(BlueprintCallable, DisplayName="Box: Is Points Inside Or On?", Category = "NEXUS|Picker|Box")
	static TArray<bool> IsPointsInsideOrOn(const TArray<FVector>& Points, const FVector& Origin, const FBox& MinimumDimensions, const FBox& MaximumDimensions)
	{
		return FNBoxPicker::IsPointsInsideOrOn(Points, Origin, MinimumDimensions, MaximumDimensions);
	}
};