// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Macros/NWorldMacros.h"
#include "NRectanglePicker.h"
#include "NRectanglePickerLibrary.generated.h"

/**
 * Blueprint function library providing access to rectangle picker functionality.
 * Exposes FNRectanglePicker methods for use in Blueprints with a simplified interface.
 */
UCLASS()
class NEXUSPICKER_API UNRectanglePickerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	// NEXT POINT   

	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Next Point (Inside Or On)", Category = "NEXUS|Picker|Rectangle")
	static FVector NextPointInsideOrOn(const FVector& Origin, const FVector2D MinimumDimensions, const FVector2D MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNRectanglePicker::NextPointInsideOrOn(ReturnLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Next Point (Inside Or On) [Simple]", Category = "NEXUS|Picker|Rectangle")
	static FVector NextPointInsideOrOnSimple(const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNRectanglePicker::NextPointInsideOrOnSimple(ReturnLocation, Origin, Dimensions, Rotation);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Next Point Projected (Inside Or On)", Category = "NEXUS|Picker|Rectangle", meta = (WorldContext = "WorldContextObject"))
	static FVector NextPointInsideOrOnProjected(const FVector& Origin, const FVector2D MinimumDimensions, const FVector2D MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNRectanglePicker::NextPointInsideOrOnProjected(ReturnLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation,
			N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Next Point Projected (Inside Or On) [Simple]", Category = "NEXUS|Picker|Rectangle", meta = (WorldContext = "WorldContextObject"))
	static FVector NextPointInsideOrOnSimpleProjected(const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNRectanglePicker::NextPointInsideOrOnSimpleProjected(ReturnLocation, Origin, Dimensions, Rotation,
			N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	// RANDOM POINT

	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random Point (Inside Or On)", Category = "NEXUS|Picker|Rectangle")
	static FVector RandomPointInsideOrOn(const FVector& Origin, const FVector2D MinimumDimensions, const FVector2D MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomPointInsideOrOn(ReturnLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random Point (Inside Or On) [Simple]", Category = "NEXUS|Picker|Rectangle")
	static FVector RandomPointInsideOrOnSimple(const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomPointInsideOrOnSimple(ReturnLocation, Origin, Dimensions, Rotation);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random Point Projected (Inside Or On)", Category = "NEXUS|Picker|Rectangle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomPointInsideOrOnProjected(const FVector& Origin, const FVector2D MinimumDimensions, const FVector2D MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomPointInsideOrOnProjected(ReturnLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation,
			N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random Point Projected (Inside Or On) [Simple]", Category = "NEXUS|Picker|Rectangle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomPointInsideOrOnSimpleProjected(const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomPointInsideOrOnSimpleProjected(ReturnLocation, Origin, Dimensions, Rotation,
			N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	// RANDOM ONE-SHOT POINT

	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random One-Shot Point (Inside Or On)", Category = "NEXUS|Picker|Rectangle")
	static FVector RandomOneShotPointInsideOrOn(const int32 Seed, const FVector& Origin, const FVector2D MinimumDimensions, const FVector2D MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomOneShotPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random One-Shot Point (Inside Or On) [Simple]", Category = "NEXUS|Picker|Rectangle")
	static FVector RandomOneShotPointInsideOrOnSimple(const int32 Seed, const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomOneShotPointInsideOrOnSimple(Seed, ReturnLocation, Origin, Dimensions, Rotation);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random One-Shot Point Projected (Inside Or On)", Category = "NEXUS|Picker|Rectangle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomOneShotPointInsideOrOnProjected(const int32 Seed, const FVector& Origin, const FVector2D MinimumDimensions, const FVector2D MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomOneShotPointInsideOrOnProjected(Seed, ReturnLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation,
			N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random One-Shot Point Projected (Inside Or On) [Simple]", Category = "NEXUS|Picker|Rectangle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomOneShotPointInsideOrOnSimpleProjected(const int32 Seed, const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomOneShotPointInsideOrOnSimpleProjected(Seed, ReturnLocation, Origin, Dimensions, Rotation,
			N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	// RANDOM TRACKED POINT

	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random Tracked Point (Inside Or On)", Category = "NEXUS|Picker|Rectangle")
	static FVector RandomTrackedPointInsideOrOn(UPARAM(ref)int32& Seed, const FVector& Origin, const FVector2D MinimumDimensions, const FVector2D MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomTrackedPointInsideOrOn(Seed, ReturnLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random Tracked Point (Inside Or On) [Simple]", Category = "NEXUS|Picker|Rectangle")
	static FVector RandomTrackedPointInsideOrOnSimple(UPARAM(ref)int32& Seed, const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation = FRotator::ZeroRotator)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomTrackedPointInsideOrOnSimple(Seed, ReturnLocation, Origin, Dimensions, Rotation);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random Tracked Point Projected (Inside Or On)", Category = "NEXUS|Picker|Rectangle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomTrackedPointInsideOrOnProjected(UPARAM(ref)int32& Seed, const FVector& Origin, const FVector2D MinimumDimensions, const FVector2D MaximumDimensions, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomTrackedPointInsideOrOnProjected(Seed, ReturnLocation, Origin, MinimumDimensions, MaximumDimensions, Rotation,
			N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	UFUNCTION(BlueprintCallable, DisplayName = "Rectangle: Random Tracked Point Projected (Inside Or On) [Simple]", Category = "NEXUS|Picker|Rectangle", meta = (WorldContext = "WorldContextObject"))
	static FVector RandomTrackedPointInsideOrOnSimpleProjected(UPARAM(ref)int32& Seed, const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation = FRotator::ZeroRotator,
		UObject* WorldContextObject = nullptr, const FVector Projection = FVector(0,0,-500.f), const ECollisionChannel CollisionChannel = ECC_WorldStatic)
	{
		FVector ReturnLocation;
		FNRectanglePicker::RandomTrackedPointInsideOrOnSimpleProjected(Seed, ReturnLocation, Origin, Dimensions, Rotation,
			N_GET_WORLD_FROM_CONTEXT(WorldContextObject), Projection, CollisionChannel);
		return ReturnLocation;
	}

	// ASSERT

	UFUNCTION(BlueprintCallable, DisplayName="Rectangle: Is Point Inside Or On?", Category = "NEXUS|Picker|Rectangle")
	static bool IsPointInsideOrOn(const FVector& Origin, const FVector2D Dimensions, const FRotator Rotation, const FVector& Point)
	{
		return FNRectanglePicker::IsPointInsideOrOn(Origin, Dimensions, Rotation, Point);
	}
};