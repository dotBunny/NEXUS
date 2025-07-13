// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NSplinePicker.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NSplinePickerLibrary.generated.h"

UCLASS()
class NEXUSPICKER_API UNSplinePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, DisplayName = "Next Point On", Category = "NEXUS|Picker|Spline")
	static FVector NextPointOn(const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::NextPointOn(ReturnLocation, SplineComponent);
		return ReturnLocation;
	}
	UFUNCTION(BlueprintCallable, DisplayName = "Next Grounded Point On", Category = "NEXUS|Picker|Spline")
	static FVector NextGroundedPointOn(const USplineComponent* SplineComponent,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::NextGroundedPointOn(ReturnLocation, SplineComponent, InWorld, CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Point On", Category = "NEXUS|Picker|Spline")
	static FVector RandomPointOn(FVector& OutLocation, const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomPointOn(ReturnLocation, SplineComponent);
		return ReturnLocation;
	}
	UFUNCTION(BlueprintCallable, DisplayName = "Random Grounded Point On", Category = "NEXUS|Picker|Spline")
	static FVector RandomGroundedPointOn(FVector& OutLocation, const USplineComponent* SplineComponent,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomGroundedPointOn(ReturnLocation, SplineComponent, InWorld, CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random One-Shot Point On", Category = "NEXUS|Picker|Spline")
	static FVector RandomOneShotPointOn(const int32 Seed, FVector& OutLocation, const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomOneShotPointOn(Seed, ReturnLocation, SplineComponent);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random One-Shot Grounded Point On", Category = "NEXUS|Picker|Spline")
	static FVector RandomOneShotGroundedPointOn(const int32 Seed, FVector& OutLocation, const USplineComponent* SplineComponent,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomOneShotGroundedPointOn(Seed, ReturnLocation, SplineComponent, InWorld, CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Tracked Point On", Category = "NEXUS|Picker|Spline")
	static FVector RandomTrackedPointOn(int32& Seed, FVector& OutLocation, const USplineComponent* SplineComponent)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomTrackedPointOn(Seed, ReturnLocation, SplineComponent);
		return ReturnLocation;
	}
	
	UFUNCTION(BlueprintCallable, DisplayName = "Random Tracked Grounded Point On", Category = "NEXUS|Picker|Spline")
	static FVector RandomTrackedGroundedPointOn(int32& Seed, FVector& OutLocation, const USplineComponent* SplineComponent,
		UWorld* InWorld = nullptr, const float CastBuffer = 500, const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSplinePicker::RandomTrackedGroundedPointOn(Seed, ReturnLocation, SplineComponent, InWorld, CastBuffer, CollisionChannel);
		return ReturnLocation;
	}
};