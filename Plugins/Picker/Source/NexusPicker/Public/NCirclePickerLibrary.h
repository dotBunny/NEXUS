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

	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Next Point (Inside Or On)", Category = "NEXUS|Picker")
	static FVector NextPointInsideOrOn(const FVector& Origin, const float MinimumRadius, const float MaximumRadius)
	{
		FVector ReturnLocation;
		FNCirclePicker::NextPointInsideOrOn(ReturnLocation, Origin, MinimumRadius, MaximumRadius);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Next Point Projected (Inside Or On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector NextPointInsideOrOnProjected(const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNCirclePicker::NextPointInsideOrOnProjected(ReturnLocation, Origin, MinimumRadius, MaximumRadius, Rotation, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Random Point (Inside Or On)", Category = "NEXUS|Picker")
	static FVector RandomPointInsideOrOn(const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomPointInsideOrOn(ReturnLocation, Origin, MinimumRadius, MaximumRadius, Rotation);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Random Point Projected (Inside Or On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomPointInsideOrOnProjected(const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomPointInsideOrOnProjected(ReturnLocation, Origin, MinimumRadius, MaximumRadius, Rotation, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Random One-Shot Point (Inside Or On)", Category = "NEXUS|Picker")
	static FVector RandomOneShotPointInsideOrOn(const int32 Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomOneShotPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, Rotation);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Random One-Shot Point Projected (Inside Or On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomOneShotPointInsideOrOnProjected(const int32 Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomOneShotPointInsideOrOnProjected(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, Rotation, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;	
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Random Tracked Point (Inside Or On)", Category = "NEXUS|Picker")
	static FVector RandomTrackedPointInsideOrOn(UPARAM(ref)int32& Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomTrackedPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, Rotation);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Circle: Random Tracked Point Projected (Inside Or On)", Category = "NEXUS|Picker", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomTrackedPointInsideOrOnProjected(UPARAM(ref)int32& Seed, const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNCirclePicker::RandomTrackedPointInsideOrOnProjected(Seed, ReturnLocation, Origin, MinimumRadius, MaximumRadius, Rotation, N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		
		return ReturnLocation;
	}
	
	UFUNCTION(BlueprintCallable, DisplayName="Circle: Is Point Inside Or On?", Category = "NEXUS|Picker")
	static bool IsPointInsideOrOn(const FVector& Origin, const float MinimumRadius, const float MaximumRadius, const FRotator Rotation, const FVector& Point)
	{
		return FNCirclePicker::IsPointInsideOrOn(Origin, MinimumRadius, MaximumRadius, Rotation, Point);
	}
};