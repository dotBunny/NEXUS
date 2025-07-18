// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NSplinePicker.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NSplinePickerLibrary.generated.h"

/**
 * Blueprint function library providing access to spline picker functionality.
 * Exposes FNSplinePicker methods for use in Blueprints with a simplified interface.
 */
UCLASS()
class NEXUSPICKER_API UNSplinePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Generates a deterministic point on a spline.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * 
	 * @param SplineComponent The spline component to generate points on.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Next Point (On)", Category = "NEXUS|Picker|Spline")
	static FVector NextPointOn(const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::NextPointOn(ReturnLocation, SplineComponent);
		return ReturnLocation;
	}

	/**
	 * Generates a deterministic point on a spline, then projects it to the world.
	 * The point is projected in the given direction until it hits something in the world.
	 * 
	 * @param SplineComponent The spline component to generate points on.
	 * @param WorldContextObject Object that provides access to the world.
	 * @param Projection Direction and distance for the line trace (default is downward 500 units).
	 * @param CollisionChannel The collision channel to use for tracing (default is WorldStatic).
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Next Point Projected (On)", Category = "NEXUS|Picker|Spline", meta = (WorldContext = "WorldContextObject"))
	static FVector NextPointOnProjected(const USplineComponent* SplineComponent,
		UObject* WorldContextObject, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::NextPointOnProjected(ReturnLocation, SplineComponent, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	/**
	 * Generates a random point on a spline.
	 * Uses the non-deterministic random generator for true randomness.
	 * 
	 * @param SplineComponent The spline component to generate points on.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random Point (On)", Category = "NEXUS|Picker|Spline")
	static FVector RandomPointOn(const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomPointOn(ReturnLocation, SplineComponent);
		return ReturnLocation;
	}

	/**
	 * Generates a random point on a spline, then projects it to the world.
	 * The point is projected in the given direction until it hits something in the world.
	 * 
	 * @param SplineComponent The spline component to generate points on.
	 * @param WorldContextObject Object that provides access to the world.
	 * @param Projection Direction and distance for the line trace (default is downward 500 units).
	 * @param CollisionChannel The collision channel to use for tracing (default is WorldStatic).
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random Point Projected (On)", Category = "NEXUS|Picker|Spline", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomPointOnProjected(const USplineComponent* SplineComponent,
		UObject* WorldContextObject, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomPointOnProjected(ReturnLocation, SplineComponent, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	/**
	 * Generates a random point on a spline using a provided seed.
	 * Useful for one-time random point generation with reproducible results.
	 * 
	 * @param Seed The random seed to use.
	 * @param SplineComponent The spline component to generate points on.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random One-Shot Point (On)", Category = "NEXUS|Picker|Spline")
	static FVector RandomOneShotPointOn(const int32 Seed, const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomOneShotPointOn(Seed, ReturnLocation, SplineComponent);
		return ReturnLocation;
	}

	/**
	 * Generates a random point on a spline using a provided seed, then projects it to the world.
	 * The point is projected in the given direction until it hits something in the world.
	 * 
	 * @param Seed The random seed to use.
	 * @param SplineComponent The spline component to generate points on.
	 * @param WorldContextObject Object that provides access to the world.
	 * @param Projection Direction and distance for the line trace (default is downward 500 units).
	 * @param CollisionChannel The collision channel to use for tracing (default is WorldStatic).
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random One-Shot Point Projected (On)", Category = "NEXUS|Picker|Spline", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomOneShotPointOnProjected(const int32 Seed, const USplineComponent* SplineComponent,
		UObject* WorldContextObject, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomOneShotPointOnProjected(Seed, ReturnLocation, SplineComponent, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	/**
	 * Generates a random point on a spline while tracking the random seed state.
	 * Updates the seed value to enable sequential random point generation.
	 * 
	 * @param Seed The random seed to use and update.
	 * @param SplineComponent The spline component to generate points on.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random Tracked Point (On)", Category = "NEXUS|Picker|Spline")
	static FVector RandomTrackedPointOn(UPARAM(ref)int32& Seed, const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomTrackedPointOn(Seed, ReturnLocation, SplineComponent);
		return ReturnLocation;
	}
	
	/**
	 * Generates a random point on a spline while tracking the random seed state, then projects it to the world.
	 * Updates the seed value to enable sequential random point generation.
	 * The point is projected in the given direction until it hits something in the world.
	 * 
	 * @param Seed The random seed to use and update.
	 * @param SplineComponent The spline component to generate points on.
	 * @param WorldContextObject Object that provides access to the world.
	 * @param Projection Direction and distance for the line trace (default is downward 500 units).
	 * @param CollisionChannel The collision channel to use for tracing (default is WorldStatic).
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random Tracked Point Projected (On)", Category = "NEXUS|Picker|Spline", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomTrackedPointOnProjected(UPARAM(ref)int32& Seed, const USplineComponent* SplineComponent,
		UObject* WorldContextObject, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomTrackedPointOnProjected(Seed, ReturnLocation, SplineComponent, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	/**
	 * Checks if a point is on a spline within a specified tolerance.
	 * Uses the N_PICKER_TOLERANCE defined in NPickerUtils.h for proximity checking.
	 * 
	 * @param SplineComponent The spline component to check against.
	 * @param Point The point to check.
	 * @return True if the point is on the spline within the tolerance, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Spline: Is Point On?", Category = "NEXUS|Picker|Spline")
	static bool IsPointOn(const USplineComponent* SplineComponent, const FVector& Point)
	{
		return FNSplinePicker::IsPointOn(SplineComponent, Point);
	}
};