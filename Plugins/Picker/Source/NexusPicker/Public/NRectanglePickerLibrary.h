// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NRectanglePicker.h"
#include "NRectanglePickerLibrary.generated.h"

/**
 * Provides various functions for generating points the plane of a rectangle using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/rectangle/">UNRectanglePickerLibrary</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS: Rectangle Picker Library")
class NEXUSPICKER_API UNRectanglePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Generates a deterministic point inside or on the boundary of a rectangle.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param Params The parameters for the point generation. 
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	 
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Next Point", Category = "NEXUS|Picker|Rectangle", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/rectangle/#next-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> NextPoint(UPARAM(ref) FNRectanglePickerParams& Params, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNRectanglePicker::Next(ReturnLocations, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the boundary of a rectangle.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param Params The parameters for the point generation. 
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	 
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random Point", Category = "NEXUS|Picker|Rectangle", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/rectangle/#random-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> RandomPoint(UPARAM(ref) FNRectanglePickerParams& Params, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNRectanglePicker::Random(ReturnLocations, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the boundary of a rectangle.
	 * Updates the seed value to enable sequential random point generation.	 
	 * @param Params The parameters for the point generation.
	 * @param Seed The random seed to start with, and update.	  
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Tracked Point", Category = "NEXUS|Picker|Rectangle", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/rectangle/#tracked-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> TrackedPoint(UPARAM(ref) FNRectanglePickerParams& Params, UPARAM(ref) int32& Seed, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNRectanglePicker::Tracked(ReturnLocations, Seed, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the boundary of a rectangle.
	 * Useful for one-time random point generation with reproducible results. 
	 * @param Params The parameters for the point generation.
	 * @param Seed The random seed to start with, and update.	  
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: One-Shot Point", Category = "NEXUS|Picker|Rectangle", 
	meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/rectangle/#one-shot-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> OneShotPoint(UPARAM(ref) FNRectanglePickerParams& Params, const int32 Seed, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNRectanglePicker::OneShot(ReturnLocations, Seed, Params);
		return MoveTemp(ReturnLocations);
	}

	/**
	 * Checks whether a given point is inside or on the boundary of a rectangle.
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
	
	/**
	 * Checks if multiple points are inside or on the boundary of a rectangle.
	 * @param Points Array of points to check.
	 * @param Origin  The center point of the rectangle.
	 * @param MinimumDimensions The minimum dimensions of the rectangle.
	 * @param MaximumDimensions The maximum dimensions of the rectangle.
	 * @param Rotation The rotation of the rectangle around its center.
	 * @return Array of booleans indicating if each point is inside or on the rectangle.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Rectangle: Is Points Inside Or On?", Category = "NEXUS|Picker|Rectangle")
	static TArray<bool> IsPointsInsideOrOn(const TArray<FVector>& Points, const FVector& Origin, const FVector2D& MinimumDimensions, const FVector2D& MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		return FNRectanglePicker::IsPointsInsideOrOn(Points, Origin, MinimumDimensions, MaximumDimensions, Rotation);
	}
};