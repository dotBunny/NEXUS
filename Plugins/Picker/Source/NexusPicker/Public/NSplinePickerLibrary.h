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
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Next Grounded Point (On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector NextGroundedPointOn(const USplineComponent* SplineComponent,
		UObject* WorldContextObject, FVector CastBuffer = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::NextGroundedPointOn(ReturnLocation, SplineComponent, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random Point (On)", Category = "NEXUS|Picker")
	static FVector RandomPointOn(const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomPointOn(ReturnLocation, SplineComponent);
		return ReturnLocation;
	}
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random Grounded Point (On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomGroundedPointOn(const USplineComponent* SplineComponent,
		UObject* WorldContextObject, FVector CastBuffer = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomGroundedPointOn(ReturnLocation, SplineComponent, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random One-Shot Point (On)", Category = "NEXUS|Picker")
	static FVector RandomOneShotPointOn(const int32 Seed, const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomOneShotPointOn(Seed, ReturnLocation, SplineComponent);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random One-Shot Grounded Point (On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomOneShotGroundedPointOn(const int32 Seed, const USplineComponent* SplineComponent,
		UObject* WorldContextObject, FVector CastBuffer = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomOneShotGroundedPointOn(Seed, ReturnLocation, SplineComponent, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random Tracked Point (On)", Category = "NEXUS|Picker")
	static FVector RandomTrackedPointOn(UPARAM(ref)int32& Seed, const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomTrackedPointOn(Seed, ReturnLocation, SplineComponent);
		return ReturnLocation;
	}
	
	UFUNCTION(BlueprintCallable, DisplayName = "Spline: Random Tracked Grounded Point (On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomTrackedGroundedPointOn(UPARAM(ref)int32& Seed, const USplineComponent* SplineComponent,
		UObject* WorldContextObject, FVector CastBuffer = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomTrackedGroundedPointOn(Seed, ReturnLocation, SplineComponent, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName="Spline: Is Point On?", Category = "NEXUS|Picker")
	static bool IsPointOn(const USplineComponent* SplineComponent, const FVector& Point)
	{
		return FNSplinePicker::IsPointOn(SplineComponent, Point);
	}

	UFUNCTION(BlueprintCallable, DisplayName="Spline: Is Point On? (Grounded)", Category = "NEXUS|Picker")
	static bool IsPointOnGrounded(const USplineComponent* SplineComponent, const FVector& Point)
	{
		return FNSplinePicker::IsPointOnGrounded(SplineComponent, Point);
	}
};