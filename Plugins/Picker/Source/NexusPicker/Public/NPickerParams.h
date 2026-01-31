// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NPickerParams.generated.h"

/**
 * Enumeration representing possible projection modes for point generation.
 */
UENUM(BlueprintType)
enum class ENPickerProjectionMode : uint8
{
	None = 0,
	Trace = 1 UMETA(DisplayName="Line Trace (Collision)"),
	NearestNavMeshV1 = 2 UMETA(DisplayName="Nearest NavMesh Point (V1)")
};

/**
 * Structure representing the parameters used for the generation of points.
 */
USTRUCT(BlueprintType)
struct NEXUSPICKER_API FNPickerParams
{
	GENERATED_BODY()

	// TODO: Is there a world where we change the logic to support Inside VS InsideOrOn when generating points
	
	/**
	 * The number of points to generate in a single pass.
	 */
	UPROPERTY(Category = "Base", BlueprintReadWrite)
	int Count = 1;
	
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
	UPROPERTY(Category = "Projection", BlueprintReadWrite, AdvancedDisplay, meta=(EditCondition="ProjectionMode==ENPickerProjectionMode::Trace"))
	FVector Projection = FVector(0,0,-500.f);

	/**
	 * The collision channel to use for tracing.
	 */
	UPROPERTY(Category = "Projection", BlueprintReadWrite, AdvancedDisplay, meta=(EditCondition="ProjectionMode==ENPickerProjectionMode::Trace"))
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_WorldStatic;
};