// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NBoxPicker.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NBoxPickerLibrary.generated.h"

UCLASS()
class NEXUSPICKER_API UNBoxPickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, DisplayName = "Box: Next Point (Inside Or On)", Category = "NEXUS|Picker")
	static FVector NextPoint(const FVector& Origin, const FBox& Dimensions)
	{
		FVector ReturnLocation;
		FNBoxPicker::NextPointInsideOrOn(ReturnLocation, Origin, Dimensions);
		return ReturnLocation;
	}
	
	UFUNCTION(BlueprintCallable, DisplayName = "Box: Next Grounded Point (Inside Or On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector NextGroundedPoint(const FVector& Origin, const FBox& Dimensions,
		UObject* WorldContextObject, FVector CastBuffer = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNBoxPicker::NextGroundedPointInsideOrOn(ReturnLocation, Origin, Dimensions, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random Point (Inside Or On)", Category = "NEXUS|Picker")
	static FVector RandomPoint(const FVector& Origin, const FBox& Dimensions)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomPointInsideOrOn(ReturnLocation, Origin, Dimensions);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random Grounded Point (Inside Or On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomGroundedPoint(const FVector& Origin, const FBox& Dimensions,
		UObject* WorldContextObject, FVector CastBuffer = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomGroundedPointInsideOrOn(ReturnLocation, Origin, Dimensions, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), CastBuffer, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random One-Shot Point (Inside Or On)", Category = "NEXUS|Picker")
	static FVector RandomOneShotPoint(const int32 Seed, const FVector& Origin, const FBox& Dimensions)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomOneShotPointInsideOrOn(Seed, ReturnLocation, Origin, Dimensions);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random One-Shot Grounded Point (Inside Or On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomOneShotGroundedPoint(const int32 Seed, const FVector& Origin, const FBox& Dimensions,
		UObject* WorldContextObject, FVector CastBuffer = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomOneShotGroundedPointInsideOrOn(Seed, ReturnLocation, Origin, Dimensions, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), CastBuffer, CollisionChannel);
		return ReturnLocation;	
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random Tracked Point (Inside Or On)", Category = "NEXUS|Picker")
	static FVector RandomTrackedPoint(UPARAM(ref)int32& Seed, const FVector& Origin, const FBox& Dimensions)
	{
		FVector ReturnLocation;
		FNBoxPicker::RandomTrackedPointInsideOrOn(Seed, ReturnLocation, Origin, Dimensions);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Box: Random Tracked Grounded Point (Inside Or On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomTrackedGroundedPoint(UPARAM(ref)int32& Seed, const FVector& Origin, const FBox& Dimensions,
		UObject* WorldContextObject, FVector CastBuffer = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		
		FVector ReturnLocation;
		FNBoxPicker::RandomTrackedGroundedPointInsideOrOn(Seed, ReturnLocation, Origin, Dimensions, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), CastBuffer, CollisionChannel);
		
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName="Box: Is Point Inside Or On?", Category = "NEXUS|Picker")
	static bool IsPointInsideOrOn(const FVector& Origin, const FBox& Dimensions, const FVector& Point)
	{
		return FNBoxPicker::IsPointInsideOrOn(Origin, Dimensions, Point);
	}
};