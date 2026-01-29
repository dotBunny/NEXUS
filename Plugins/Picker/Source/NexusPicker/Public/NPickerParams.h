// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerParams.generated.h"

UENUM(BlueprintType)
enum class ENPickerProjectionMode : uint8
{
	None = 0,
	Projected = 1,
	NearestNavMesh = 2
};

USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNPickerParams
{
	GENERATED_BODY()

	// InsideOrOn vs Inside vs On
	
	/**
	 * The number of points to generate in a single pass.
	 */
	UPROPERTY(Category = "Base", BlueprintReadWrite)
	int Count = 1;

	/**
	 * The center point when attempting to generate new points.
	 */
	UPROPERTY(Category = "Base", BlueprintReadWrite)
	FVector Origin = FVector::ZeroVector;
	
	/**
	 * The world for line tracing.
	 */
	UPROPERTY(Category = "Base", BlueprintReadOnly, VisibleInstanceOnly)
	TObjectPtr<UWorld> CachedWorld = nullptr;

	/**
	 * Should the point be projected somewhere?
	 */
	UPROPERTY(Category = "Projection", BlueprintReadWrite, AdvancedDisplay)
	ENPickerProjectionMode ProjectionMode = ENPickerProjectionMode::None;

	/**
	 * Direction and distance for the line trace.
	 */
	UPROPERTY(Category = "Projection", BlueprintReadWrite, AdvancedDisplay, meta=(EditCondition="ProjectionMode==ENPickerProjectionMode::Projection"))
	FVector Projection = FVector(0,0,-500.f);

	/**
	 * The collision channel to use for tracing.
	 */
	UPROPERTY(Category = "Projection", BlueprintReadWrite, AdvancedDisplay, meta=(EditCondition="ProjectionMode==ENPickerProjectionMode::Projection"))
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_WorldStatic;
};