// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCirclePicker.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NCirclePickerLibrary.generated.h"

UCLASS()
class NEXUSPICKER_API UNCirclePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, DisplayName = "Next Point (Inside Or On)", Category = "NEXUS|Picker|Circle")
	static FVector NextPoint(const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNCirclePicker::NextPointInsideOrOn(ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Next Grounded Point (Inside Or On)", Category = "NEXUS|Picker|Circle", meta = (WorldContext = "WorldContextObject"))
	static FVector NextGroundedPoint(const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UObject* WorldContextObject, FVector CastBuffer = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNCirclePicker::NextGroundedPointInsideOrOn(ReturnLocation, Origin, MinimumRadius, MaximumRadius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Point (Inside Or On)", Category = "NEXUS|Picker|Circle")
	static FVector RandomPoint(const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomPointInsideOrOn(ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Grounded Point (Inside Or On)", Category = "NEXUS|Picker|Circle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomGroundedPoint(const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UObject* WorldContextObject, FVector CastBuffer = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomGroundedPointInsideOrOn(ReturnLocation, Origin, MinimumRadius, MaximumRadius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random One-Shot Point (Inside Or On)", Category = "NEXUS|Picker|Circle")
	static FVector RandomOneShotPoint(const int32 Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomOneShotPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Random One-Shot Grounded Point (Inside Or On)", Category = "NEXUS|Picker|Circle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomOneShotGroundedPoint(const int32 Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UObject* WorldContextObject, FVector CastBuffer = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomOneShotGroundedPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), CastBuffer, CollisionChannel);
		return ReturnLocation;	
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Random Tracked Point (Inside Or On)", Category = "NEXUS|Picker|Circle")
	static FVector RandomTrackedPoint(int32& Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomOneShotPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Tracked Grounded Point (Inside Or On)", Category = "NEXUS|Picker|Circle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomTrackedGroundedPoint(int32& Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UObject* WorldContextObject, FVector CastBuffer = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomTrackedGroundedPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), CastBuffer, CollisionChannel);
		
		return ReturnLocation;
	}
};