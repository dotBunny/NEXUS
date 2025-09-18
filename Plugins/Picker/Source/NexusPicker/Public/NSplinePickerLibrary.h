// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NSplinePicker.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NSplinePickerLibrary.generated.h"

/**
 * Provides various functions for generating points along a USplineComponent spline using different
 * random generation strategies (deterministic, non-deterministic, seeded).
 * @see <a href="https://nexus-framework.com/docs/plugins/picker/distributions/spline/">UNSplinePickerLibrary</a>
 */
UCLASS()
class NEXUSPICKER_API UNSplinePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Generates a deterministic point on a spline.
	 * @param SplineComponent The spline component to generate points on.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Next Point (On)", Category = "NEXUS|Picker|Spline",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/spline/#next-point-io"))
	static FVector NextPointOn(const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::NextPointOn(ReturnLocation, SplineComponent);
		return ReturnLocation;
	}

	/**
	 * Generates a deterministic point on a spline, then projects it to the world.
	 * @param SplineComponent The spline component to generate points on.
	 * @param Projection Direction and distance for the line trace (default is down 500 units).
	 * @param CollisionChannel The collision channel to use for tracing (default is WorldStatic).
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Next Point Projected (On)", Category = "NEXUS|Picker|Spline",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/spline/#next-point-projected-on"))
	static FVector NextPointOnProjected(const USplineComponent* SplineComponent,
		const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		
		FVector ReturnLocation;
		FNSplinePicker::NextPointOnProjected(ReturnLocation, SplineComponent, SplineComponent->GetWorld(), Projection, CollisionChannel);
		return ReturnLocation;
	}

	/**
	 * Generates a random point on a spline.
	 * @param SplineComponent The spline component to generate points on.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random Point (On)", Category = "NEXUS|Picker|Spline",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/spline/#random-point-on"))
	static FVector RandomPointOn(const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomPointOn(ReturnLocation, SplineComponent);
		return ReturnLocation;
	}

	/**
	 * Generates a random point on a spline, then projects it to the world.
	 * @param SplineComponent The spline component to generate points on.
	 * @param Projection Direction and distance for the line trace (default is down 500 units).
	 * @param CollisionChannel The collision channel to use for tracing (default is WorldStatic).
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random Point Projected (On)", Category = "NEXUS|Picker|Spline",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/spline/#random-point-projected-on"))
	static FVector RandomPointOnProjected(const USplineComponent* SplineComponent,
		const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomPointOnProjected(ReturnLocation, SplineComponent, SplineComponent->GetWorld(), Projection, CollisionChannel);
		return ReturnLocation;
	}

	/**
	 * Generates a random point on a spline using a provided seed.
	 * @param Seed The random seed to use.
	 * @param SplineComponent The spline component to generate points on.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random One-Shot Point (On)", Category = "NEXUS|Picker|Spline",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/spline/#random-point-projected-on"))
	static FVector RandomOneShotPointOn(const int32 Seed, const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomOneShotPointOn(Seed, ReturnLocation, SplineComponent);
		return ReturnLocation;
	}

	/**
	 * Generates a random point on a spline using a provided seed, then projects it to the world.
	 * @param Seed The random seed to use.
	 * @param SplineComponent The spline component to generate points on.
	 * @param Projection Direction and distance for the line trace (default is down 500 units).
	 * @param CollisionChannel The collision channel to use for tracing (default is WorldStatic).
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random One-Shot Point Projected (On)", Category = "NEXUS|Picker|Spline",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/spline/#random-one-shot-point-projected-on"))
	static FVector RandomOneShotPointOnProjected(const int32 Seed, const USplineComponent* SplineComponent,
		const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomOneShotPointOnProjected(Seed, ReturnLocation, SplineComponent, SplineComponent->GetWorld(), Projection, CollisionChannel);
		return ReturnLocation;
	}

	/**
	 * Generates a random point on a spline while tracking the random seed state.
	 * @param Seed The random seed to use and update.
	 * @param SplineComponent The spline component to generate points on.
	 * @return The generated point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random Tracked Point (On)", Category = "NEXUS|Picker|Spline",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/spline/#random-tracked-point-on"))
	static FVector RandomTrackedPointOn(UPARAM(ref)int32& Seed, const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomTrackedPointOn(Seed, ReturnLocation, SplineComponent);
		return ReturnLocation;
	}
	
	/**
	 * Generates a random point on a spline while tracking the random seed state, then projects it to the world.
	 * @param Seed The random seed to use and update.
	 * @param SplineComponent The spline component to generate points on.
	 * @param Projection Direction and distance for the line trace (default is down 500 units).
	 * @param CollisionChannel The collision channel to use for tracing (default is WorldStatic).
	 * @return The generated and projected point location.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random Tracked Point Projected (On)", Category = "NEXUS|Picker|Spline",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/picker/distributions/spline/#random-tracked-point-projected-on"))
	static FVector RandomTrackedPointOnProjected(UPARAM(ref)int32& Seed, const USplineComponent* SplineComponent,
		const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomTrackedPointOnProjected(Seed, ReturnLocation, SplineComponent, SplineComponent->GetWorld(), Projection, CollisionChannel);
		return ReturnLocation;
	}

	/**
	 * Checks if a point is on a spline within a specified tolerance.
	 * Uses the N_PICKER_TOLERANCE defined in NPickerUtils.h for proximity checking.
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