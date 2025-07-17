// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NSplinePicker.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NSplinePickerLibrary.generated.h"

UCLASS()
class NEXUSPICKER_API UNSplinePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Next Point (On)", Category = "NEXUS|Picker")
	static FVector NextPointOn(const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::NextPointOn(ReturnLocation, SplineComponent);
		return ReturnLocation;
	}
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Next Point Projected (On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector NextPointOnProjected(const USplineComponent* SplineComponent,
		UObject* WorldContextObject, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::NextPointOnProjected(ReturnLocation, SplineComponent, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random Point (On)", Category = "NEXUS|Picker")
	static FVector RandomPointOn(const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomPointOn(ReturnLocation, SplineComponent);
		return ReturnLocation;
	}
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random Point Projected (On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomPointOnProjected(const USplineComponent* SplineComponent,
		UObject* WorldContextObject, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomPointOnProjected(ReturnLocation, SplineComponent, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random One-Shot Point (On)", Category = "NEXUS|Picker")
	static FVector RandomOneShotPointOn(const int32 Seed, const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomOneShotPointOn(Seed, ReturnLocation, SplineComponent);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random One-Shot Point Projected (On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomOneShotPointOnProjected(const int32 Seed, const USplineComponent* SplineComponent,
		UObject* WorldContextObject, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomOneShotPointOnProjected(Seed, ReturnLocation, SplineComponent, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random Tracked Point (On)", Category = "NEXUS|Picker")
	static FVector RandomTrackedPointOn(UPARAM(ref)int32& Seed, const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomTrackedPointOn(Seed, ReturnLocation, SplineComponent);
		return ReturnLocation;
	}
	
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random Tracked Point Projected (On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomTrackedPointOnProjected(UPARAM(ref)int32& Seed, const USplineComponent* SplineComponent,
		UObject* WorldContextObject, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomTrackedPointOnProjected(Seed, ReturnLocation, SplineComponent, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName="Spline: Is Point On?", Category = "NEXUS|Picker")
	static bool IsPointOn(const USplineComponent* SplineComponent, const FVector& Point)
	{
		return FNSplinePicker::IsPointOn(SplineComponent, Point);
	}

	// TODO: REMOVE
	/*
	 * @notes This is extremely 
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Spline: Is Point On? (Grounded)", Category = "NEXUS|Picker")
	static bool IsPointOnGrounded(const USplineComponent* SplineComponent, const FVector& Point)
	{
		return FNSplinePicker::IsPointOnGrounded(SplineComponent, Point);
	}
};