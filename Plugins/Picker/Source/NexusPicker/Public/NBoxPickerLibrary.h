// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NBoxPicker.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NBoxPickerLibrary.generated.h"

/**
 * A Blueprint Function Library that provides utilities for picking points inside or on an axis-aligned FBox.
 * 
 * This library exposes functionality from the FNBoxPicker class to blueprints, allowing for:
 * - Deterministic point selection (via Next functions)
 * - Random point selection (with various seeding methods)
 * - Projection capabilities for point placement
 * - Point validation within a FBox
 */
UCLASS()
class NEXUSPICKER_API UNBoxPickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Gets the next deterministic point inside or on a FBox.
	 * 
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @return A point that is inside or on the FBox.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Box: Next Point (Inside Or On)", Category = "NEXUS|Picker")
	static FVector NextPointInsideOrOn(const FVector& Origin, const FBox& Dimensions)
	{
		FVector ReturnLocation;
		FNBoxPicker::NextPointInsideOrOn(ReturnLocation, Origin, Dimensions);
		return ReturnLocation;
	}

	/**
	 * Gets a random point inside or on a FBox.
	 * 
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @return A random point that is inside or on the FBox.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random Point (Inside Or On)", Category = "NEXUS|Picker")
	static FVector RandomPointInsideOrOn(const FVector& Origin, const FBox& Dimensions)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomPointInsideOrOn(ReturnLocation, Origin, Dimensions);
		return ReturnLocation;
	}

	/**
	 * Gets a random point inside or on a FBox using a one-shot seed.
	 * 
	 * @param Seed The seed value to use for the random number generation.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @return A random point that is inside or on the FBox.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random One-Shot Point (Inside Or On)", Category = "NEXUS|Picker")
	static FVector RandomOneShotPointInsideOrOn(const int32 Seed, const FVector& Origin, const FBox& Dimensions)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomOneShotPointInsideOrOn(Seed, ReturnLocation, Origin, Dimensions);
		return ReturnLocation;
	}

	/**
	 * Gets a random point inside or on a FBox using a tracked seed.
	 * The seed is incremented each time this function is called.
	 * 
	 * @param Seed The seed value used for random number generation. Will be incremented after use.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @return A random point that is inside or on the FBox.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random Tracked Point (Inside Or On)", Category = "NEXUS|Picker")
	static FVector RandomTrackedPointInsideOrOn(UPARAM(ref)int32& Seed, const FVector& Origin, const FBox& Dimensions)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomTrackedPointInsideOrOn(Seed, ReturnLocation, Origin, Dimensions);
		return ReturnLocation;
	}

	/**
	 * Gets the next deterministic point inside or on a FBox and projects it onto the ground.
	 * 
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @param WorldContextObject Object that provides access to the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the trace check.
	 * @return A grounded point that is inside or on the FBox.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Box: Next Point Projected (Inside Or On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector NextPointInsideOrOnProjected(const FVector& Origin, const FBox& Dimensions,
		UObject* WorldContextObject, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNBoxPicker::NextPointInsideOrOnProjected(ReturnLocation, Origin, Dimensions, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}
	
	/**
	 * Gets a random point inside or on a FBox and projects it onto the ground.
	 * 
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @param WorldContextObject Object that provides access to the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the trace check.
	 * @return A random grounded point that is inside or on the FBox.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random Point Projected (Inside Or On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomPointInsideOrOnProjected(const FVector& Origin, const FBox& Dimensions,
		UObject* WorldContextObject, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomPointInsideOrOnProjected(ReturnLocation, Origin, Dimensions, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}
	
	/**
	 * Gets a random point inside or on a FBox using a one-shot seed and projects it onto the ground.
	 * 
	 * @param Seed The seed value to use for the random number generation.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @param WorldContextObject Object that provides access to the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the trace check.
	 * @return A random grounded point that is inside or on the FBox.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random One-Shot Point Projected (Inside Or On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomOneShotPointInsideOrOnProjected(const int32 Seed, const FVector& Origin, const FBox& Dimensions,
		UObject* WorldContextObject, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomOneShotPointInsideOrOnProjected(Seed, ReturnLocation, Origin, Dimensions, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;	
	}
	
	/**
	 * Gets a random point inside or on a FBox using a tracked seed and projects it onto the ground.
	 * The seed is incremented each time this function is called.
	 * 
	 * @param Seed The seed value used for random number generation. Will be incremented after use.
	 * @param Origin The center point of the FBox.
	 * @param Dimensions The dimensions of the FBox.
	 * @param WorldContextObject Object that provides access to the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the trace check.
	 * @return A random grounded point that is inside or on the FBox.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random Tracked Point Projected (Inside Or On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomTrackedPointInsideOrOnProjected(UPARAM(ref)int32& Seed, const FVector& Origin, const FBox& Dimensions,
		UObject* WorldContextObject, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		
		FVector ReturnLocation;
		FNBoxPicker::RandomTrackedPointInsideOrOnProjected(Seed, ReturnLocation, Origin, Dimensions, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		
		return ReturnLocation;
	}
	
	/**
	 * Checks if a point is inside or on a FBox.
	 * 
	 * @param Origin      The center point of the FBox.
	 * @param Dimensions  The dimensions of the FBox.
	 * @param Point       The point to check.
	 * @return            True if the point is inside or on the FBox, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Box: Is Point Inside Or On?", Category = "NEXUS|Picker")
	static bool IsPointInsideOrOn(const FVector& Origin, const FBox& Dimensions, const FVector& Point)
	{
		return FNBoxPicker::IsPointInsideOrOn(Origin, Dimensions, Point);
	}
};