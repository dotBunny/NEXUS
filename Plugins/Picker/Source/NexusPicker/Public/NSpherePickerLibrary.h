// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NSpherePicker.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NSpherePickerLibrary.generated.h"


/**
 * Provides various functions for generating points inside or on the surface of a sphere using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/">UNSpherePickerLibrary</a>
 */
UCLASS(DisplayName = "NEXUS: Sphere Picker Library")
class NEXUSPICKER_API UNSpherePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	// NEXT POINT
	
	/**
	 * Generates a deterministic point inside or on the surface of a sphere.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Next Point (IO)", Category = "NEXUS|Picker|Sphere",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#next-point-io"))
	static FVector NextPointInsideOrOn(const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNSpherePicker::NextPointInsideOrOn(ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}
	/**
	 * Generates a deterministic point inside or on the surface of a sphere.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Next Point (IO) [Simple]", Category = "NEXUS|Picker|Sphere",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#next-point-io"))
	static FVector NextPointInsideOrOnSimple(const FVector& Origin, const float Radius)
	{
		FVector ReturnLocation;
		FNSpherePicker::NextPointInsideOrOnSimple(ReturnLocation, Origin, Radius);
		return ReturnLocation;
	}
	/**
	 * Generates a deterministic point inside or on the surface of a sphere, then projects it onto a surface in the world.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Next Point Projected (IO)", Category = "NEXUS|Picker|Sphere", meta = (WorldContext = "WorldContextObject",
		DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#next-point-projected-io"))
	static FVector NextPointInsideOrOnProjected(const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UObject* WorldContextObject, FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::NextPointInsideOrOnProjected(ReturnLocation, Origin, MinimumRadius, MaximumRadius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}
	/**
	 * Generates a deterministic point inside or on the surface of a sphere, then projects it onto a surface in the world.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Next Point Projected (IO) [Simple]", Category = "NEXUS|Picker|Sphere", meta = (WorldContext = "WorldContextObject",
		DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#next-point-projected-io"))
	static FVector NextPointInsideOrOnSimpleProjected(const FVector& Origin, const float Radius,
		UObject* WorldContextObject, FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::NextPointInsideOrOnSimpleProjected(ReturnLocation, Origin, Radius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}


	// RANDOM POINT
	
	/**
	 * Generates a random point inside or on the surface of a sphere.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random Point (IO)", Category = "NEXUS|Picker|Sphere",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#random-point-io"))
	static FVector RandomPointInsideOrOn(const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomPointInsideOrOn(ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}
	/**
	 * Generates a random point inside or on the surface of a sphere.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random Point (IO) [Simple]", Category = "NEXUS|Picker|Sphere",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#random-point-io"))
	static FVector RandomPointInsideOrOnSimple(const FVector& Origin, const float Radius)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomPointInsideOrOnSimple(ReturnLocation, Origin, Radius);
		return ReturnLocation;
	}
	/**
	 * Generates a random point inside or on the surface of a sphere, then projects it onto a surface in the world.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random Point Projected (IO)", Category = "NEXUS|Picker|Sphere", meta = (WorldContext = "WorldContextObject",
		DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#random-point-projected-io"))
	static FVector RandomPointInsideOrOnProjected(const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UObject* WorldContextObject, FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomPointInsideOrOnProjected(ReturnLocation, Origin, MinimumRadius, MaximumRadius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the surface of a sphere, then projects it onto a surface in the world.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random Point Projected (IO) [Simple]", Category = "NEXUS|Picker|Sphere", meta = (WorldContext = "WorldContextObject",
		DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#random-point-projected-io"))
	static FVector RandomPointInsideOrOnSimpleProjected(const FVector& Origin, const float Radius,
		UObject* WorldContextObject, FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomPointInsideOrOnSimpleProjected(ReturnLocation, Origin, Radius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	// RANDOM ONE-SHOT POINT
	
	/**
	 * Generates a random point inside or on the surface of a sphere using a provided seed.
	 * @param Seed The random seed to use.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random One-Shot Point (IO)", Category = "NEXUS|Picker|Sphere",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#random-one-shot-point-io"))
	static FVector RandomOneShotPointInsideOrOn(const int32 Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomOneShotPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the surface of a sphere using a provided seed.
	 * @param Seed The random seed to use.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random One-Shot Point (IO) [Simple]", Category = "NEXUS|Picker|Sphere",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#random-one-shot-point-io"))
	static FVector RandomOneShotPointInsideOrOnSimple(const int32 Seed, const FVector& Origin, const float Radius)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomOneShotPointInsideOrOnSimple(Seed, ReturnLocation, Origin, Radius);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the surface of a sphere using a provided seed, then projects it onto a surface in the world.
	 * @param Seed The random seed to use.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random One-Shot Point Projected (IO)", Category = "NEXUS|Picker|Sphere", meta = (WorldContext = "WorldContextObject",
		DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#random-one-shot-point-projected-io"))
	static FVector RandomOneShotPointInsideOrOnProjected(const int32 Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UObject* WorldContextObject, FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomOneShotPointInsideOrOnProjected(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;	
	}

	/**
	 * Generates a random point inside or on the surface of a sphere using a provided seed, then projects it onto a surface in the world.
	 * @param Seed The random seed to use.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random One-Shot Point Projected (IO) [Simple]", Category = "NEXUS|Picker|Sphere", meta = (WorldContext = "WorldContextObject",
		DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#random-one-shot-point-projected-io"))
	static FVector RandomOneShotPointInsideOrOnSimpleProjected(const int32 Seed, const FVector& Origin, const float Radius,
		UObject* WorldContextObject, FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomOneShotPointInsideOrOnSimpleProjected(Seed, ReturnLocation, Origin, Radius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;	
	}

	// RANDOM TRACKED POINT
	
	/**
	 * Generates a random point inside or on the surface of a sphere while tracking the random seed state.
	 * @param Seed [in,out] The random seed to use and update.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random Tracked Point (IO)", Category = "NEXUS|Picker|Sphere",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#random-tracked-point-io"))
	static FVector RandomTrackedPointInsideOrOn(UPARAM(ref)int32& Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomTrackedPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the surface of a sphere while tracking the random seed state.
	 * @param Seed [in,out] The random seed to use and update.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random Tracked Point (IO) [Simple]", Category = "NEXUS|Picker|Sphere",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#random-tracked-point-io"))
	static FVector RandomTrackedPointInsideOrOnSimple(UPARAM(ref)int32& Seed, const FVector& Origin, const float Radius)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomTrackedPointInsideOrOnSimple(Seed, ReturnLocation, Origin, Radius);
		return ReturnLocation;
	}

	/**
	 * Generates a random point inside or on the surface of a sphere while tracking the random seed state, then projects it onto a surface in the world.
	 * @param Seed [in,out] The random seed to use and update.
	 * @param Origin The center point of the sphere.
	 * @param MinimumRadius The minimum radius of the sphere (inner bound).
	 * @param MaximumRadius The maximum radius of the sphere (outer bound).
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random Tracked Projected Point (IO)", Category = "NEXUS|Picker|Sphere", meta = (WorldContext = "WorldContextObject",
		DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#random-tracked-point-projected-io"))
	static FVector RandomTrackedPointInsideOrOnProjected(UPARAM(ref)int32& Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UObject* WorldContextObject, FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomTrackedPointInsideOrOnProjected(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		
		return ReturnLocation;
	}
	/**
	 * Generates a random point inside or on the surface of a sphere while tracking the random seed state, then projects it onto a surface in the world.
	 * @param Seed [in,out] The random seed to use and update.
	 * @param Origin The center point of the sphere.
	 * @param Radius The radius of the sphere.
	 * @param WorldContextObject Object that provides context for accessing the world.
	 * @param Projection Direction and distance of the projection line trace (default is downward).
	 * @param CollisionChannel The collision channel to use for the line trace.
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Sphere: Random Tracked Projected Point (IO) [Simple]", Category = "NEXUS|Picker|Sphere", meta = (WorldContext = "WorldContextObject",
		DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/sphere/#random-tracked-point-projected-io"))
	static FVector RandomTrackedPointInsideOrOnSimpleProjected(UPARAM(ref)int32& Seed, const FVector& Origin, const float Radius,
		UObject* WorldContextObject, FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomTrackedPointInsideOrOnSimpleProjected(Seed, ReturnLocation, Origin, Radius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		
		return ReturnLocation;
	}

	// ASSERT
	
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
};