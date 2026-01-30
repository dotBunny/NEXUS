// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCirclePicker.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NCirclePickerLibrary.generated.h"

/**
 * Provides various functions for generating points in the plane of a circle using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/circle/">UNCirclePickerLibrary</a>
 * @note There are no simple variants to these methods as it wouldn't change the computation.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS: Circle Picker Library")
class NEXUSPICKER_API UNCirclePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Generates a deterministic point inside or on the perimeter of a circle.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param Params The parameters for the point generation. 
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	 
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Next Point", Category = "NEXUS|Picker|Circle", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/box/#next-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> NextPoint(UPARAM(ref) FNCirclePickerParams& Params, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNCirclePicker::Next(ReturnLocations, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the perimeter of a circle.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param Params The parameters for the point generation. 
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	 
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Random Point", Category = "NEXUS|Picker|Circle", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/circle/#random-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> RandomPoint(UPARAM(ref) FNCirclePickerParams& Params, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNCirclePicker::Random(ReturnLocations, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the perimeter of a circle.
	 * Updates the seed value to enable sequential random point generation.	 
	 * @param Params The parameters for the point generation.
	 * @param Seed The random seed to start with, and update.	  
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Tracked Point", Category = "NEXUS|Picker|Circle", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/circle/#tracked-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> TrackedPoint(UPARAM(ref) FNCirclePickerParams& Params, UPARAM(ref) int32& Seed, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNCirclePicker::Tracked(ReturnLocations, Seed, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the perimeter of a circle.
	 * Useful for one-time random point generation with reproducible results. 
	 * @param Params The parameters for the point generation.
	 * @param Seed The random seed to start with, and update.	  
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "Circle: One-Shot Point", Category = "NEXUS|Picker|Circle", 
	meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/circle/#one-shot-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> OneShotPoint(UPARAM(ref) FNCirclePickerParams& Params, const int32 Seed, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNCirclePicker::OneShot(ReturnLocations, Seed, Params);
		return MoveTemp(ReturnLocations);
	}

	/**
	 * Checks if a point is inside or on the perimeter of a circle.
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation The rotation of the circle plane.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the perimeter of the circle, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Circle: Is Point Inside Or On?", Category = "NEXUS|Picker|Circle")
	static bool IsPointInsideOrOn(const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation, const FVector& Point)
	{
		return FNCirclePicker::IsPointInsideOrOn(Origin, MinimumRadius, MaximumRadius, Rotation, Point);
	}
	
	/**
	 * Checks if multiple points are inside or on the perimeter of a circle.
	 * @param Points The array of points to check.
	 * @param Origin The center point of the FBox.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation The rotation of the circle plane.
	 * @return An array of boolean values indicating if each point is inside or on the perimeter of a circle.
	 */	
	UFUNCTION(BlueprintCallable, DisplayName="Circle: Is Points Inside Or On?", Category = "NEXUS|Picker|Circle")
	static TArray<bool> IsPointsInsideOrOn(const TArray<FVector>& Points, const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation)
	{
		return FNCirclePicker::IsPointsInsideOrOn(Points, Origin, MinimumRadius, MaximumRadius, Rotation);
	}
};