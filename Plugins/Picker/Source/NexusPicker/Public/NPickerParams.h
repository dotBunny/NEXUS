// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerParams.generated.h"

USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNPickerParams
{
	GENERATED_BODY()	
	
	UPROPERTY(Category = "Base", BlueprintReadWrite, AdvancedDisplay)
	int Count = 1;

	/**
	 * The center point when attempting to generate new points.
	 */
	UPROPERTY(Category = "Base", BlueprintReadWrite)
	FVector Origin = FVector::ZeroVector;
	
	/**
	 * The world for line tracing.
	 */
	UPROPERTY(Category = "Base", BlueprintReadWrite)
	TObjectPtr<UWorld> World = nullptr;
	
	
	
	UPROPERTY(Category = "Projection", BlueprintReadWrite)
	bool bProjectPoint = false;

	/**
	 * Direction and distance for the line trace.
	 */
	UPROPERTY(Category = "Projection", BlueprintReadWrite)
	FVector Projection = FVector(0,0,-500.f);

	/**
	 * The collision channel to use for tracing.
	 */
	UPROPERTY(Category = "Projection", BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_WorldStatic;
};