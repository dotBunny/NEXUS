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
 */
UCLASS()
class NEXUSPICKER_API UNCirclePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Generates a deterministic point inside or on the perimeter of a circle.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * 
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation Optional rotation to apply to the circle plane (default is ZeroRotator).
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Next Point (IO)", Category = "NEXUS|Picker|Circle")
	static FVector NextPointInsideOrOn(const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNCirclePicker::NextPointInsideOrOn(ReturnLocation, Origin, MinimumRadius, MaximumRadius, Rotation);
		return ReturnLocation;
	}

	/**
	 * Generates a deterministic point inside or on the perimeter of a circle, then projects it to the world.
	 * The point is projected in the given direction until it hits something in the world.
	 * 
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation Optional rotation to apply to the circle plane (default is ZeroRotator).
	 * @param WorldContextObject Object that provides access to the world.
	 * @param Projection Direction and distance for the line trace (default is downward 500 units).
	 * @param CollisionChannel The collision channel to use for tracing (default is WorldStatic).
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Next Point Projected (IO)", Category = "NEXUS|Picker|Circle", meta = (WorldContext = "WorldContextObject"))
	static FVector NextPointInsideOrOnProjected(const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNCirclePicker::NextPointInsideOrOnProjected(ReturnLocation, Origin, MinimumRadius, MaximumRadius, Rotation, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the perimeter of a circle.
	 * Uses the non-deterministic random generator for true randomness.
	 * 
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation Optional rotation to apply to the circle plane (default is ZeroRotator).
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Random Point (IO)", Category = "NEXUS|Picker|Circle")
	static FVector RandomPointInsideOrOn(const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomPointInsideOrOn(ReturnLocation, Origin, MinimumRadius, MaximumRadius, Rotation);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the perimeter of a circle, then projects it to the world.
	 * The point is projected in the given direction until it hits something in the world.
	 * 
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation Optional rotation to apply to the circle plane (default is ZeroRotator).
	 * @param WorldContextObject Object that provides access to the world.
	 * @param Projection Direction and distance for the line trace (default is downward 500 units).
	 * @param CollisionChannel The collision channel to use for tracing (default is WorldStatic).
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Random Point Projected (IO)", Category = "NEXUS|Picker|Circle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomPointInsideOrOnProjected(const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomPointInsideOrOnProjected(ReturnLocation, Origin, MinimumRadius, MaximumRadius, Rotation, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the perimeter of a circle using a provided seed.
	 * Useful for one-time random point generation with reproducible results.
	 * 
	 * @param Seed The random seed to use.
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation Optional rotation to apply to the circle plane (default is ZeroRotator).
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Random One-Shot Point (IO)", Category = "NEXUS|Picker|Circle")
	static FVector RandomOneShotPointInsideOrOn(const int32 Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomOneShotPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, Rotation);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the perimeter of a circle using a provided seed, then projects it to the world.
	 * The point is projected in the given direction until it hits something in the world.
	 * 
	 * @param Seed The random seed to use.
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation Optional rotation to apply to the circle plane (default is ZeroRotator).
	 * @param WorldContextObject Object that provides access to the world.
	 * @param Projection Direction and distance for the line trace (default is downward 500 units).
	 * @param CollisionChannel The collision channel to use for tracing (default is WorldStatic).
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Random One-Shot Point Projected (IO)", Category = "NEXUS|Picker|Circle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomOneShotPointInsideOrOnProjected(const int32 Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomOneShotPointInsideOrOnProjected(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, Rotation, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;	
	}

	/**
	 * Generates a random point inside or on the perimeter of a circle while tracking the random seed state.
	 * Updates the seed value to enable sequential random point generation.
	 * 
	 * @param Seed The random seed to use and update.
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation Optional rotation to apply to the circle plane (default is ZeroRotator).
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Random Tracked Point (IO)", Category = "NEXUS|Picker|Circle")
	static FVector RandomTrackedPointInsideOrOn(UPARAM(ref)int32& Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomTrackedPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, Rotation);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the perimeter of a circle while tracking the random seed state, then projects it to the world.
	 * Updates the seed value to enable sequential random point generation.
	 * The point is projected in the given direction until it hits something in the world.
	 * 
	 * @param Seed The random seed to use and update.
	 * @param Origin The center point of the circle.
	 * @param MinimumRadius The minimum radius of the circle (inner bound).
	 * @param MaximumRadius The maximum radius of the circle (outer bound).
	 * @param Rotation Optional rotation to apply to the circle plane (default is ZeroRotator).
	 * @param WorldContextObject Object that provides access to the world.
	 * @param Projection Direction and distance for the line trace (default is downward 500 units).
	 * @param CollisionChannel The collision channel to use for tracing (default is WorldStatic).
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Random Tracked Point Projected (IO)", Category = "NEXUS|Picker|Circle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomTrackedPointInsideOrOnProjected(UPARAM(ref)int32& Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomTrackedPointInsideOrOnProjected(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, Rotation, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		
		return ReturnLocation;
	}
	
	/**
	 * Checks if a point is inside or on the perimeter of a circle.
	 * 
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
};