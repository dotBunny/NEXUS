// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NSpherePicker.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NSpherePickerLibrary.generated.h"


/**
 * A Blueprint Function Library that provides utilities for picking points inside or on a Sphere.
 * 
 * This library exposes functionality from the FNSpherePicker class to blueprints, allowing for:
 * - Deterministic point selection (via Next functions)
 * - Random point selection (with various seeding methods)
 * - Projection capabilities for point placement
 * - Point validation within a Sphere
 */
UCLASS()
class NEXUSPICKER_API UNSpherePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Generates a deterministic point inside or on the surface of a sphere.
	 * Will always return the same point for identical input parameters.
	 * 
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Next Point (Inside Or On)", Category = "NEXUS|Picker")
	static FVector NextPointInsideOrOn(const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNSpherePicker::NextPointInsideOrOn(ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	/**
	 * Generates a deterministic point inside or on the surface of a sphere, then projects it onto a surface in the world.
	 * The projection is a line trace that starts at the generated point and extends along the specified direction.
	 * 
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Next Point Projected (Inside Or On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector NextPointInsideOrOnProjected(const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UObject* WorldContextObject, FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::NextPointInsideOrOnProjected(ReturnLocation, Origin, MinimumRadius, MaximumRadius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the surface of a sphere.
	 * Uses the non-deterministic random generator for true randomness.
	 * 
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random Point (Inside Or On)", Category = "NEXUS|Picker")
	static FVector RandomPointInsideOrOn(const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomPointInsideOrOn(ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the surface of a sphere, then projects it onto a surface in the world.
	 * The projection is a line trace that starts at the generated point and extends along the specified direction.
	 * 
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random Point Projected (Inside Or On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomPointInsideOrOnProjected(const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UObject* WorldContextObject, FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomPointInsideOrOnProjected(ReturnLocation, Origin, MinimumRadius, MaximumRadius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the surface of a sphere using a provided seed.
	 * Useful for one-time random point generation with reproducible results.
	 * 
	 * @param Seed The random seed to use.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random One-Shot Point (Inside Or On)", Category = "NEXUS|Picker")
	static FVector RandomOneShotPointInsideOrOn(const int32 Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomOneShotPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the surface of a sphere using a provided seed, then projects it onto a surface in the world.
	 * The projection is a line trace that starts at the generated point and extends along the specified direction.
	 * 
	 * @param Seed The random seed to use.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random One-Shot Point Projected (Inside Or On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomOneShotPointInsideOrOnProjected(const int32 Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UObject* WorldContextObject, FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomOneShotPointInsideOrOnProjected(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;	
	}

	/**
	 * Generates a random point inside or on the surface of a sphere while tracking the random seed state.
	 * Updates the seed value to enable sequential random point generation with different results.
	 * 
	 * @param Seed [in,out] The random seed to use and update.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random Tracked Point (Inside Or On)", Category = "NEXUS|Picker")
	static FVector RandomTrackedPointInsideOrOn(UPARAM(ref)int32& Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomTrackedPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the surface of a sphere while tracking the random seed state, then projects it onto a surface in the world.
	 * Updates the seed value to enable sequential random point generation with different results.
	 * The projection is a line trace that starts at the generated point and extends along the specified direction.
	 * 
	 * @param Seed [in,out] The random seed to use and update.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random Tracked Projected Point (Inside Or On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomTrackedPointInsideOrOnProjected(UPARAM(ref)int32& Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UObject* WorldContextObject, FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomTrackedPointInsideOrOnProjected(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		
		return ReturnLocation;
	}

	/**
	 * Checks if a point is inside or on the surface of a sphere.
	 * 
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the surface of the sphere, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Sphere: Is Point Inside Or On?", Category = "NEXUS|Picker")
	static bool IsPointInsideOrOn(const FVector& Origin, const float Radius, const FVector& Point)
	{
		return FNSpherePicker::IsPointInsideOrOn(Origin, Radius, Point);
	}
};