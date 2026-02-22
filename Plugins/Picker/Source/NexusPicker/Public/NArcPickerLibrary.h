// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NArcPicker.h"
#include "NArcPickerLibrary.generated.h"

/**
 * Provides various functions for generating points the plane of an arc using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/arc/">UNArcPickerLibrary</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS: Arc Picker Library")
class NEXUSPICKER_API UNArcPickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Generates a deterministic point inside or on the boundary of an arc.
	 * Uses the deterministic random generator to ensure reproducible results.
	 * @param Params The parameters for the point generation. 
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	 
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Arc: Next Point", Category = "NEXUS|Picker|Arc", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/arc/#next-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> NextPoint(UPARAM(ref) FNArcPickerParams& Params, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNArcPicker::Next(ReturnLocations, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the boundary of an arc.
	 * Uses the non-deterministic random generator for true randomness.
	 * @param Params The parameters for the point generation. 
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	 
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Arc: Random Point", Category = "NEXUS|Picker|Arc", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/arc/#random-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> RandomPoint(UPARAM(ref) FNArcPickerParams& Params, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNArcPicker::Random(ReturnLocations, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the boundary of an arc.
	 * Updates the seed value to enable sequential random point generation.	 
	 * @param Params The parameters for the point generation.
	 * @param Seed The random seed to start with, and update.	  
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Arc: Tracked Point", Category = "NEXUS|Picker|Arc", 
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/arc/#tracked-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> TrackedPoint(UPARAM(ref) FNArcPickerParams& Params, UPARAM(ref) int32& Seed, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNArcPicker::Tracked(ReturnLocations, Seed, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Generates a deterministic point inside or on the boundary of an arc.
	 * Useful for one-time random point generation with reproducible results. 
	 * @param Params The parameters for the point generation.
	 * @param Seed The random seed to start with, and update.	  
	 * @param WorldContextObject Object that provides access to the world.
	 * @returns An array of generated points.	
	 */	
	UFUNCTION(BlueprintCallable, DisplayName = "Arc: One-Shot Point", Category = "NEXUS|Picker|Arc", 
	meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/arc/#one-shot-point", WorldContext = "WorldContextObject"))
	static TArray<FVector> OneShotPoint(UPARAM(ref) FNArcPickerParams& Params, const int32 Seed, UObject* WorldContextObject)
	{
		TArray<FVector> ReturnLocations;
		if (Params.CachedWorld == nullptr)
		{
			Params.CachedWorld = N_GET_WORLD_FROM_CONTEXT(WorldContextObject);	
		}
		FNArcPicker::OneShot(ReturnLocations, Seed, Params);
		return MoveTemp(ReturnLocations);
	}
	
	/**
	 * Checks if a point is inside or on the surface of an arc.
	 * @param Origin The center point of the FBox.
	 * @param Rotation The base rotation of the arc.
	 * @param Degrees The angle of the arc in degrees.
	 * @param MinimumDistance The minimum distance of the arc.
	 * @param MaximumDistance The maximum distance of the arc.
	 * @param Point The point to check.
	 * @return True if the point is inside or on the surface of an arc, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Arc: Is Point Inside Or On?", Category = "NEXUS|Picker|Arc")
	static bool IsPointInsideOrOn(const FVector& Origin, const FRotator& Rotation, const float& Degrees, const float& MinimumDistance, const float& MaximumDistance, const FVector& Point)
	{
		return FNArcPicker::IsPointInsideOrOn(Origin, Rotation, Degrees, MinimumDistance, MaximumDistance, Point);
	}

	/**
	 * Checks if multiple points are inside or on the surface of an arc.
	 * @param Points The array of points to check.
	 * @param Origin The center point of an arc.
	 * @param Rotation The base rotation of the arc.
	 * @param Degrees The angle of the arc in degrees.
	 * @param MinimumDistance The minimum distance of the arc.
	 * @param MaximumDistance The maximum distance of the arc.
	 * @return An array of boolean values indicating if each point is inside or on the surface of an arc.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Arc: Is Points Inside Or On?", Category = "NEXUS|Picker|Arc")
	static TArray<bool> IsPointsInsideOrOn(const TArray<FVector>& Points, const FVector& Origin, const FRotator& Rotation, const float& Degrees, const float& MinimumDistance, const float& MaximumDistance)
	{
		return FNArcPicker::IsPointsInsideOrOn(Points, Origin, Rotation, Degrees, MinimumDistance, MaximumDistance);
	}
};