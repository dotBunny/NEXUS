// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NOrientedBoxPicker.h"
#include "NOrientedBoxPickerLibrary.generated.h"

/**
 * Provides various functions for generating points the plane of a FOrientedBox using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/oriented-box/">UNOrientedBoxPickerLibrary</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS: OrientedBox Picker Library")
class NEXUSPICKER_API UNOrientedBoxPickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	/**
	 * Generates a deterministic point inside or on the boundary of a FOrientedBox.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param Params The parameters for the point generation. 
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	 
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "OrientedBox: Next Point", Category = "NEXUS|Picker|OrientedBox", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/oriented-box/#next-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> NextPoint(UPARAM(ref) FNOrientedBoxPickerParams& Params, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNOrientedBoxPicker::Next(ReturnLocations, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the boundary of a FOrientedBox.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param Params The parameters for the point generation. 
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	 
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "OrientedBox: Random Point", Category = "NEXUS|Picker|OrientedBox", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/oriented-box/#random-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> RandomPoint(UPARAM(ref) FNOrientedBoxPickerParams& Params, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNOrientedBoxPicker::Random(ReturnLocations, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the boundary of a FOrientedBox.
	 * Updates the seed value to enable sequential random point generation.	 
	 * @param Params The parameters for the point generation.
	 * @param Seed The random seed to start with, and update.	  
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "OrientedBox: Tracked Point", Category = "NEXUS|Picker|OrientedBox", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/oriented-box/#tracked-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> TrackedPoint(UPARAM(ref) FNOrientedBoxPickerParams& Params, UPARAM(ref) int32& Seed, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNOrientedBoxPicker::Tracked(ReturnLocations, Seed, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the boundary of a FOrientedBox.
	 * Useful for one-time random point generation with reproducible results. 
	 * @param Params The parameters for the point generation.
	 * @param Seed The random seed to start with, and update.	  
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "OrientedBox: One-Shot Point", Category = "NEXUS|Picker|OrientedBox", 
	meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/oriented-box/#one-shot-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> OneShotPoint(UPARAM(ref) FNOrientedBoxPickerParams& Params, const int32 Seed, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNOrientedBoxPicker::OneShot(ReturnLocations, Seed, Params);
		return MoveTemp(ReturnLocations);
	}

	/**
	 * Checks whether a given point is inside or on the boundary of a FOrientedBox.
	 * @param Origin The center point of the FOrientedBox.
	 * @param Dimensions The dimensions of the FOrientedBox.
	 * @param Rotation The rotation of the FOrientedBox around its center.
	 * @param Point The point to test.
	 * @return True if the point is inside or on the boundary of the FOrientedBox, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="OrientedBox: Is Point Inside Or On?", Category = "NEXUS|Picker|OrientedBox")
	static bool IsPointInsideOrOn(const FVector& Origin, const FVector& Dimensions, const FRotator& Rotation, const FVector& Point)
	{
		return FNOrientedBoxPicker::IsPointInsideOrOn(Origin, Dimensions, Rotation, Point);
	}
	
	/**
	 * Checks if multiple points are inside or on the boundary of a FOrientedBox.
	 * @param Points Array of points to check.
	 * @param Origin  The center point of the FOrientedBox.
	 * @param MinimumDimensions The minimum dimensions of the FOrientedBox.
	 * @param MaximumDimensions The maximum dimensions of the FOrientedBox.
	 * @param Rotation The rotation of the FOrientedBox around its center.
	 * @return Array of booleans indicating if each point is inside or on the FOrientedBox.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="OrientedBox: Is Points Inside Or On?", Category = "NEXUS|Picker|OrientedBox")
	static TArray<bool> IsPointsInsideOrOn(const TArray<FVector>& Points, const FVector& Origin, const FVector& MinimumDimensions, const FVector& MaximumDimensions, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		return FNOrientedBoxPicker::IsPointsInsideOrOn(Points, Origin, MinimumDimensions, MaximumDimensions, Rotation);
	}

	/**
	 * Creates a FNOrientedBoxPickerParams initialized with the properties of a FOrientedBox.
	 * @param OrientedBox The FOrientedBox to initialize the parameters from.
	 * @return The initialized FNOrientedBoxPickerParams.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "OrientedBox: Initialize Params", Category = "NEXUS|Picker|OrientedBox")
	static FNOrientedBoxPickerParams InitializeParms(const FOrientedBox& OrientedBox)
	{
		FNOrientedBoxPickerParams Params;
		Params.InitializeFrom(OrientedBox);
		return MoveTemp(Params);
	}
};