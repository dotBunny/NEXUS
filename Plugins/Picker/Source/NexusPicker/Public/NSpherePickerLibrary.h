// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NSpherePicker.h"
#include "NSpherePickerParams.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NSpherePickerLibrary.generated.h"


/**
 * Provides various functions for generating points inside or on the surface of a sphere using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/">UNSpherePickerLibrary</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS: Sphere Picker Library")
class NEXUSPICKER_API UNSpherePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Generates a deterministic point inside or on the surface of a sphere.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param Params The parameters for the point generation. 
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	 
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Next Point", Category = "NEXUS|Picker|Sphere", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#next-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> NextPoint(UPARAM(ref) FNSpherePickerParams& Params, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNSpherePicker::Next(ReturnLocations, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the surface of a sphere.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param Params The parameters for the point generation. 
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	 
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random Point", Category = "NEXUS|Picker|Sphere", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#random-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> RandomPoint(UPARAM(ref) FNSpherePickerParams& Params, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNSpherePicker::Random(ReturnLocations, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the surface of a sphere.
	 * Updates the seed value to enable sequential random point generation.	 
	 * @param Params The parameters for the point generation.
	 * @param Seed The random seed to start with, and update.	  
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Tracked Point", Category = "NEXUS|Picker|Sphere", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#tracked-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> TrackedPoint(UPARAM(ref) FNSpherePickerParams& Params, UPARAM(ref) int32& Seed, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNSpherePicker::Tracked(ReturnLocations, Seed, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the surface of a sphere.
	 * Useful for one-time random point generation with reproducible results. 
	 * @param Params The parameters for the point generation.
	 * @param Seed The random seed to start with, and update.	  
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: One-Shot Point", Category = "NEXUS|Picker|Sphere", 
	meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#one-shot-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> OneShotPoint(UPARAM(ref) FNSpherePickerParams& Params, const int32 Seed, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNSpherePicker::OneShot(ReturnLocations, Seed, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Checks if a point is inside or on the surface of a sphere.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the surface of the sphere, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Sphere: Is Point Inside Or On?", Category = "NEXUS|Picker|Sphere")
	static bool IsPointInsideOrOn(const FVector& Origin, const float Radius, const FVector& Point)
	{
		return FNSpherePicker::IsPointInsideOrOn(Origin, Radius, Point);
	}
	
	/**
	 * Checks if multiple points are inside or on the surface of a sphere
	 * @param Points Array of points to check.
	 * @param Origin  The center point of the rectangle.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (inner bound).
	 * @return Array of booleans indicating if each point is inside or on the surface of a sphere
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Sphere: Is Points Inside Or On?", Category = "NEXUS|Picker|Sphere")
	static TArray<bool> IsPointsInsideOrOn(const TArray<FVector>& Points, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		return FNSpherePicker::IsPointsInsideOrOn(Points, Origin, MinimumRadius, MaximumRadius);
	}
};