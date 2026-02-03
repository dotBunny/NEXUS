// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NRadialPicker.h"
#include "NRadialPickerLibrary.generated.h"

/**
 * Provides various functions for generating points the plane of a Radial using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/radial/">UNRadialPickerLibrary</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS: Radial Picker Library")
class NEXUSPICKER_API UNRadialPickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Generates a deterministic point inside or on the boundary of a Radial.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param Params The parameters for the point generation. 
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	 
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Radial: Next Point", Category = "NEXUS|Picker|Radial", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/radial/#next-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> NextPoint(UPARAM(ref) FNRadialPickerParams& Params, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNRadialPicker::Next(ReturnLocations, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the boundary of a Radial.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param Params The parameters for the point generation. 
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	 
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Radial: Random Point", Category = "NEXUS|Picker|Radial", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/radial/#random-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> RandomPoint(UPARAM(ref) FNRadialPickerParams& Params, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNRadialPicker::Random(ReturnLocations, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the boundary of a Radial.
	 * Updates the seed value to enable sequential random point generation.	 
	 * @param Params The parameters for the point generation.
	 * @param Seed The random seed to start with, and update.	  
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Radial: Tracked Point", Category = "NEXUS|Picker|Radial", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/radial/#tracked-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> TrackedPoint(UPARAM(ref) FNRadialPickerParams& Params, UPARAM(ref) int32& Seed, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNRadialPicker::Tracked(ReturnLocations, Seed, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the boundary of a Radial.
	 * Useful for one-time random point generation with reproducible results. 
	 * @param Params The parameters for the point generation.
	 * @param Seed The random seed to start with, and update.	  
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "Radial: One-Shot Point", Category = "NEXUS|Picker|Radial", 
	meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/radial/#one-shot-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> OneShotPoint(UPARAM(ref) FNRadialPickerParams& Params, const int32 Seed, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNRadialPicker::OneShot(ReturnLocations, Seed, Params);
		return MoveTemp(ReturnLocations);
	}
};