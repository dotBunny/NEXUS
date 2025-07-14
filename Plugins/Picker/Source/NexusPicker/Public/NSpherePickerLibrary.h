// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NSpherePicker.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NSpherePickerLibrary.generated.h"

UCLASS()
class NEXUSPICKER_API UNSpherePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, DisplayName = "Next Point (Inside Or On)", Category = "NEXUS|Picker|Sphere")
	static FVector NextPoint(const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNSpherePicker::NextPointInsideOrOn(ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Next Grounded Point (Inside Or On)", Category = "NEXUS|Picker|Sphere", meta = (WorldContext = "WorldContextObject"))
	static FVector NextGroundedPoint(const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UObject* WorldContextObject, FVector CastBuffer = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::NextGroundedPointInsideOrOn(ReturnLocation, Origin, MinimumRadius, MaximumRadius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Point (Inside Or On)", Category = "NEXUS|Picker|Sphere")
	static FVector RandomPoint(const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomPointInsideOrOn(ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Grounded Point (Inside Or On)", Category = "NEXUS|Picker|Sphere", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomGroundedPoint(const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UObject* WorldContextObject, FVector CastBuffer = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomGroundedPointInsideOrOn(ReturnLocation, Origin, MinimumRadius, MaximumRadius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random One-Shot Point (Inside Or On)", Category = "NEXUS|Picker|Sphere")
	static FVector RandomOneShotPoint(const int32 Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomOneShotPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random One-Shot Grounded Point (Inside Or On)", Category = "NEXUS|Picker|Sphere", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomOneShotGroundedPoint(const int32 Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UObject* WorldContextObject, FVector CastBuffer = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomOneShotGroundedPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), CastBuffer, CollisionChannel);
		return ReturnLocation;	
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Tracked Point (Inside Or On)", Category = "NEXUS|Picker|Sphere")
	static FVector RandomTrackedPoint(int32& Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomOneShotPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Random Tracked Grounded Point (Inside Or On)", Category = "NEXUS|Picker|Sphere", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomTrackedGroundedPoint(int32& Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius,
		UObject* WorldContextObject, FVector CastBuffer = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNSpherePicker::RandomTrackedGroundedPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), CastBuffer, CollisionChannel);
		
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName="Is Point Inside Or On?", Category = "NEXUS|Picker|Sphere")
	static bool IsPointInsideOrOn(const FVector& Origin, const float Radius, const FVector& Point)
	{
		return FNSpherePicker::IsPointInsideOrOn(Origin, Radius, Point);
	}
};