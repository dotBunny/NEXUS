// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NRectanglePicker.h"
#include "NRectanglePickerLibrary.generated.h"

/**
 * A Blueprint Function Library that provides utilities for picking points inside or on a Rectangle.
 * 
 * This library exposes functionality from the FNRectanglePicker class to blueprints, allowing for:
 * - Deterministic point selection (via Next functions)
 * - Random point selection (with various seeding methods)
 * - Projection capabilities for point placement
 * - Point validation within a Rectangle
 * - Support for both minimum/maximum dimensions and simple dimension variants
 * - Rotation support for oriented rectangles
 */
UCLASS()
class NEXUSPICKER_API UNRectanglePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	// NEXT POINT   

	/**
	 * Generates a deterministic point inside or on the boundary of a rectangle.
	 * Will always return the same point for identical input parameters.
	 * 
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum dimensions of the rectangle (X = width, Y = height).
	 * @param MaximumDimensions The maximum dimensions of the rectangle (X = width, Y = height).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Next Point (Inside Or On)", Category = "NEXUS|Picker|Rectangle")
	static FVector NextPointInsideOrOn(const FVector& Origin, const FVector2D MinimumDimensions, const FVector2D MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNRectanglePicker::NextPointInsideOrOn(ReturnLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation);
		return ReturnLocation;
	}

	/**
	 * Generates a deterministic point inside or on the boundary of a rectangle using simple dimensions.
	 * Will always return the same point for identical input parameters.
	 * 
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The dimensions of the rectangle (X = width, Y = height).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Next Point (Inside Or On) [Simple]", Category = "NEXUS|Picker|Rectangle")
	static FVector NextPointInsideOrOnSimple(const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNRectanglePicker::NextPointInsideOrOnSimple(ReturnLocation, Origin, Dimensions, Rotation);
		return ReturnLocation;
	}

	/**
	 * Generates a deterministic point inside or on the boundary of a rectangle, then projects it onto a surface in the world.
	 * The projection is a line trace that starts at the generated point and extends along the specified direction.
	 * 
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum dimensions of the rectangle (X = width, Y = height).
	 * @param MaximumDimensions The maximum dimensions of the rectangle (X = width, Y = height).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Next Point Projected (Inside Or On)", Category = "NEXUS|Picker|Rectangle", meta = (WorldContext = "WorldContextObject"))
	static FVector NextPointInsideOrOnProjected(const FVector& Origin, const FVector2D MinimumDimensions, const FVector2D MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNRectanglePicker::NextPointInsideOrOnProjected(ReturnLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation,
			N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	/**
	 * Generates a deterministic point inside or on the boundary of a rectangle using simple dimensions, then projects it onto a surface in the world.
	 * The projection is a line trace that starts at the generated point and extends along the specified direction.
	 * 
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The dimensions of the rectangle (X = width, Y = height).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Next Point Projected (Inside Or On) [Simple]", Category = "NEXUS|Picker|Rectangle", meta = (WorldContext = "WorldContextObject"))
	static FVector NextPointInsideOrOnSimpleProjected(const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNRectanglePicker::NextPointInsideOrOnSimpleProjected(ReturnLocation, Origin, Dimensions, Rotation,
			N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	// RANDOM POINT

	/**
	 * Generates a random point inside or on the boundary of a rectangle.
	 * Uses the non-deterministic random generator for true randomness.
	 * 
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum dimensions of the rectangle (X = width, Y = height).
	 * @param MaximumDimensions The maximum dimensions of the rectangle (X = width, Y = height).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random Point (Inside Or On)", Category = "NEXUS|Picker|Rectangle")
	static FVector RandomPointInsideOrOn(const FVector& Origin, const FVector2D MinimumDimensions, const FVector2D MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomPointInsideOrOn(ReturnLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the boundary of a rectangle using simple dimensions.
	 * Uses the non-deterministic random generator for true randomness.
	 * 
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The dimensions of the rectangle (X = width, Y = height).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random Point (Inside Or On) [Simple]", Category = "NEXUS|Picker|Rectangle")
	static FVector RandomPointInsideOrOnSimple(const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomPointInsideOrOnSimple(ReturnLocation, Origin, Dimensions, Rotation);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the boundary of a rectangle, then projects it onto a surface in the world.
	 * The projection is a line trace that starts at the generated point and extends along the specified direction.
	 * 
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum dimensions of the rectangle (X = width, Y = height).
	 * @param MaximumDimensions The maximum dimensions of the rectangle (X = width, Y = height).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random Point Projected (Inside Or On)", Category = "NEXUS|Picker|Rectangle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomPointInsideOrOnProjected(const FVector& Origin, const FVector2D MinimumDimensions, const FVector2D MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomPointInsideOrOnProjected(ReturnLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation,
			N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the boundary of a rectangle using simple dimensions, then projects it onto a surface in the world.
	 * The projection is a line trace that starts at the generated point and extends along the specified direction.
	 * 
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The dimensions of the rectangle (X = width, Y = height).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random Point Projected (Inside Or On) [Simple]", Category = "NEXUS|Picker|Rectangle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomPointInsideOrOnSimpleProjected(const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomPointInsideOrOnSimpleProjected(ReturnLocation, Origin, Dimensions, Rotation,
			N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	// RANDOM ONE-SHOT POINT

	/**
	 * Generates a random point inside or on the boundary of a rectangle using a provided seed.
	 * Useful for one-time random point generation with reproducible results.
	 * 
	 * @param Seed The random seed to use.
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum dimensions of the rectangle (X = width, Y = height).
	 * @param MaximumDimensions The maximum dimensions of the rectangle (X = width, Y = height).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random One-Shot Point (Inside Or On)", Category = "NEXUS|Picker|Rectangle")
	static FVector RandomOneShotPointInsideOrOn(const int32 Seed, const FVector& Origin, const FVector2D MinimumDimensions, const FVector2D MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomOneShotPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the boundary of a rectangle using simple dimensions and a provided seed.
	 * Useful for one-time random point generation with reproducible results.
	 * 
	 * @param Seed The random seed to use.
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The dimensions of the rectangle (X = width, Y = height).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random One-Shot Point (Inside Or On) [Simple]", Category = "NEXUS|Picker|Rectangle")
	static FVector RandomOneShotPointInsideOrOnSimple(const int32 Seed, const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomOneShotPointInsideOrOnSimple(Seed, ReturnLocation, Origin, Dimensions, Rotation);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the boundary of a rectangle using a provided seed, then projects it onto a surface in the world.
	 * The projection is a line trace that starts at the generated point and extends along the specified direction.
	 * 
	 * @param Seed The random seed to use.
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum dimensions of the rectangle (X = width, Y = height).
	 * @param MaximumDimensions The maximum dimensions of the rectangle (X = width, Y = height).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random One-Shot Point Projected (Inside Or On)", Category = "NEXUS|Picker|Rectangle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomOneShotPointInsideOrOnProjected(const int32 Seed, const FVector& Origin, const FVector2D MinimumDimensions, const FVector2D MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomOneShotPointInsideOrOnProjected(Seed, ReturnLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation,
			N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the boundary of a rectangle using simple dimensions and a provided seed, then projects it onto a surface in the world.
	 * The projection is a line trace that starts at the generated point and extends along the specified direction.
	 * 
	 * @param Seed The random seed to use.
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The dimensions of the rectangle (X = width, Y = height).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random One-Shot Point Projected (Inside Or On) [Simple]", Category = "NEXUS|Picker|Rectangle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomOneShotPointInsideOrOnSimpleProjected(const int32 Seed, const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomOneShotPointInsideOrOnSimpleProjected(Seed, ReturnLocation, Origin, Dimensions, Rotation,
			N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	// RANDOM TRACKED POINT

	/**
	 * Generates a random point inside or on the boundary of a rectangle using a tracked seed.
	 * The seed is modified after each call, allowing for sequences of random points.
	 * 
	 * @param Seed The random seed to use and modify (passed by reference).
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum dimensions of the rectangle (X = width, Y = height).
	 * @param MaximumDimensions The maximum dimensions of the rectangle (X = width, Y = height).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random Tracked Point (Inside Or On)", Category = "NEXUS|Picker|Rectangle")
	static FVector RandomTrackedPointInsideOrOn(UPARAM(ref)int32& Seed, const FVector& Origin, const FVector2D MinimumDimensions, const FVector2D MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomTrackedPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the boundary of a rectangle using simple dimensions and a tracked seed.
	 * The seed is modified after each call, allowing for sequences of random points.
	 * 
	 * @param Seed The random seed to use and modify (passed by reference).
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The dimensions of the rectangle (X = width, Y = height).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random Tracked Point (Inside Or On) [Simple]", Category = "NEXUS|Picker|Rectangle")
	static FVector RandomTrackedPointInsideOrOnSimple(UPARAM(ref)int32& Seed, const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomTrackedPointInsideOrOnSimple(Seed, ReturnLocation, Origin, Dimensions, Rotation);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the boundary of a rectangle using a tracked seed, then projects it onto a surface in the world.
	 * The seed is modified after each call, allowing for sequences of random points.
	 * The projection is a line trace that starts at the generated point and extends along the specified direction.
	 * 
	 * @param Seed The random seed to use and modify (passed by reference).
	 * @param Origin The center point of the rectangle.
	 * @param MinimumDimensions The minimum dimensions of the rectangle (X = width, Y = height).
	 * @param MaximumDimensions The maximum dimensions of the rectangle (X = width, Y = height).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random Tracked Point Projected (Inside Or On)", Category = "NEXUS|Picker|Rectangle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomTrackedPointInsideOrOnProjected(UPARAM(ref)int32& Seed, const FVector& Origin, const FVector2D MinimumDimensions, const FVector2D MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomTrackedPointInsideOrOnProjected(Seed, ReturnLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation,
			N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the boundary of a rectangle using simple dimensions and a tracked seed, then projects it onto a surface in the world.
	 * The seed is modified after each call, allowing for sequences of random points.
	 * The projection is a line trace that starts at the generated point and extends along the specified direction.
	 * 
	 * @param Seed The random seed to use and modify (passed by reference).
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The dimensions of the rectangle (X = width, Y = height).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random Tracked Point Projected (Inside Or On) [Simple]", Category = "NEXUS|Picker|Rectangle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomTrackedPointInsideOrOnSimpleProjected(UPARAM(ref)int32& Seed, const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomTrackedPointInsideOrOnSimpleProjected(Seed, ReturnLocation, Origin, Dimensions, Rotation,
			N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	// ASSERT

	/**
	 * Checks whether a given point is inside or on the boundary of a rectangle.
	 * 
	 * @param Origin The center point of the rectangle.
	 * @param Dimensions The dimensions of the rectangle (X = width, Y = height).
	 * @param Rotation The rotation of the rectangle around its center.
	 * @param Point The point to test.
	 * @return True if the point is inside or on the boundary of the rectangle, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Rectangle: Is Point Inside Or On?", Category = "NEXUS|Picker|Rectangle")
	static bool IsPointInsideOrOn(const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation, const FVector& Point)
	{
		return FNRectanglePicker::IsPointInsideOrOn(Origin, Dimensions, Rotation, Point);
	}
};