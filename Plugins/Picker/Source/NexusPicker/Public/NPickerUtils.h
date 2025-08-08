// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "CollisionQueryParams.h"

NEXUSPICKER_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusPicker, Log, All);

#define N_PICKER_TOLERANCE 0.001f

#define N_VLOG_PICKER_INNER_COLOR FColor::Black
#define N_VLOG_PICKER_OUTER_COLOR FColor::White
#define N_VLOG_PICKER_POINT_COLOR FColor::Green
#define N_VLOG_PICKER_POINT_SIZE 2.f
#define N_VLOG_PICKER_CHANNEL LogNexusPicker

#define N_VARIABLES_PICKER_PROJECTION() \
	const UWorld* InWorld = nullptr, const FVector& Projection = FNPickerUtils::DefaultProjection, const ECollisionChannel CollisionChannel = ECollisionChannel::ECC_WorldStatic

#define N_IMPLEMENT_PICKER_PROJECTION() \
	FHitResult HitResult(ForceInit); \
	if (InWorld->LineTraceSingleByChannel(HitResult, OutLocation, (OutLocation + Projection), CollisionChannel, FNPickerUtils::DefaultTraceParams)) \
	{ \
		OutLocation = HitResult.Location; \
	}

#if ENABLE_VISUAL_LOG

// BOX

#define N_IMPLEMENT_VLOG_BOX() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled) { \
		UE_VLOG_WIREBOX(World, N_VLOG_PICKER_CHANNEL, Verbose, MinimumDimensions.MoveTo(Origin), N_VLOG_PICKER_INNER_COLOR, TEXT("")); \
		UE_VLOG_WIREBOX(World, N_VLOG_PICKER_CHANNEL, Verbose, MaximumDimensions.MoveTo(Origin), N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(World, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_BOX_SIMPLE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled) { \
		UE_VLOG_WIREBOX(World, N_VLOG_PICKER_CHANNEL, Verbose, Dimensions.MoveTo(Origin), N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(World, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_BOX_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled) { \
		UE_VLOG_WIREBOX(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, MinimumDimensions.MoveTo(Origin), N_VLOG_PICKER_INNER_COLOR, TEXT("")); \
		UE_VLOG_WIREBOX(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, MaximumDimensions.MoveTo(Origin), N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_BOX_SIMPLE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled) { \
		UE_VLOG_WIREBOX(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, Dimensions.MoveTo(Origin), N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

// CIRCLE

#define N_IMPLEMENT_VLOG_CIRCLE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled) { \
		UE_VLOG_WIRECIRCLE(World, N_VLOG_PICKER_CHANNEL, Verbose, Origin, Rotation.RotateVector(FVector::UpVector), MinimumRadius, N_VLOG_PICKER_INNER_COLOR, TEXT("")); \
		UE_VLOG_WIRECIRCLE(World, N_VLOG_PICKER_CHANNEL, Verbose, Origin, Rotation.RotateVector(FVector::UpVector), MaximumRadius, N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(World, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_CIRCLE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled) { \
		UE_VLOG_WIRECIRCLE(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, Origin, Rotation.RotateVector(FVector::UpVector), MinimumRadius, N_VLOG_PICKER_INNER_COLOR, TEXT("")); \
		UE_VLOG_WIRECIRCLE(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, Origin, Rotation.RotateVector(FVector::UpVector), MaximumRadius, N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

// RECTANGLE

#define N_IMPLEMENT_VLOG_RECTANGLE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled) { \
		TArray<FVector> MinimumPoints; \
		MinimumPoints.Reserve(4); \
		const float MinExtentX = MinimumDimensions.X * 0.5f; \
		const float MinExtentY = MinimumDimensions.Y * 0.5f; \
		MinimumPoints.Add(Origin + Rotation.RotateVector(FVector(-MinExtentX, -MinExtentY, 0))); \
		MinimumPoints.Add(Origin + Rotation.RotateVector(FVector(-MinExtentX, MinExtentY, 0))); \
		MinimumPoints.Add(Origin + Rotation.RotateVector(FVector(MinExtentX, MinExtentY, 0))); \
		MinimumPoints.Add(Origin + Rotation.RotateVector(FVector(MinExtentX, -MinExtentY, 0))); \
		UE_VLOG_CONVEXPOLY(World, N_VLOG_PICKER_CHANNEL, Verbose, MinimumPoints, N_VLOG_PICKER_INNER_COLOR, TEXT("")); \
		TArray<FVector> MaximumPoints; \
		MaximumPoints.Reserve(4);\
		const float MaxExtentX = MaximumDimensions.X * 0.5f; \
		const float MaxExtentY = MaximumDimensions.Y * 0.5f; \
		MaximumPoints.Add(Origin + Rotation.RotateVector(FVector(-MaxExtentX, -MaxExtentY, 0))); \
		MaximumPoints.Add(Origin + Rotation.RotateVector(FVector(-MaxExtentX, MaxExtentY, 0))); \
		MaximumPoints.Add(Origin + Rotation.RotateVector(FVector(MaxExtentX, MaxExtentY, 0))); \
		MaximumPoints.Add(Origin + Rotation.RotateVector(FVector(MaxExtentX, -MaxExtentY, 0))); \
		UE_VLOG_CONVEXPOLY(World, N_VLOG_PICKER_CHANNEL, Verbose, MaximumPoints, N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(World, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_RECTANGLE_SIMPLE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled) { \
		TArray<FVector> Points; \
		Points.Reserve(4); \
		const float MinExtentX = Dimensions.X * 0.5f; \
		const float MinExtentY = Dimensions.Y * 0.5f; \
		Points.Add(Origin + Rotation.RotateVector(FVector(-MinExtentX, -MinExtentY, 0))); \
		Points.Add(Origin + Rotation.RotateVector(FVector(-MinExtentX, MinExtentY, 0))); \
		Points.Add(Origin + Rotation.RotateVector(FVector(MinExtentX, MinExtentY, 0))); \
		Points.Add(Origin + Rotation.RotateVector(FVector(MinExtentX, -MinExtentY, 0))); \
		UE_VLOG_CONVEXPOLY(World, N_VLOG_PICKER_CHANNEL, Verbose, Points, N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(World, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_RECTANGLE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled) { \
		TArray<FVector> MinimumPoints; \
		MinimumPoints.Reserve(4); \
		const float MinExtentX = MinimumDimensions.X * 0.5f; \
		const float MinExtentY = MinimumDimensions.Y * 0.5f; \
		MinimumPoints.Add(Origin + Rotation.RotateVector(FVector(-MinExtentX, -MinExtentY, 0))); \
		MinimumPoints.Add(Origin + Rotation.RotateVector(FVector(-MinExtentX, MinExtentY, 0))); \
		MinimumPoints.Add(Origin + Rotation.RotateVector(FVector(MinExtentX, MinExtentY, 0))); \
		MinimumPoints.Add(Origin + Rotation.RotateVector(FVector(MinExtentX, -MinExtentY, 0))); \
		UE_VLOG_CONVEXPOLY(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, MinimumPoints, N_VLOG_PICKER_INNER_COLOR, TEXT("")); \
		TArray<FVector> MaximumPoints; \
		MaximumPoints.Reserve(4);\
		const float MaxExtentX = MaximumDimensions.X * 0.5f; \
		const float MaxExtentY = MaximumDimensions.Y * 0.5f; \
		MaximumPoints.Add(Origin + Rotation.RotateVector(FVector(-MaxExtentX, -MaxExtentY, 0))); \
		MaximumPoints.Add(Origin + Rotation.RotateVector(FVector(-MaxExtentX, MaxExtentY, 0))); \
		MaximumPoints.Add(Origin + Rotation.RotateVector(FVector(MaxExtentX, MaxExtentY, 0))); \
		MaximumPoints.Add(Origin + Rotation.RotateVector(FVector(MaxExtentX, -MaxExtentY, 0))); \
		UE_VLOG_CONVEXPOLY(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, MaximumPoints, N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_RECTANGLE_SIMPLE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled) { \
		TArray<FVector> Points; \
		Points.Reserve(4); \
		const float MinExtentX = Dimensions.X * 0.5f; \
		const float MinExtentY = Dimensions.Y * 0.5f; \
		Points.Add(Origin + Rotation.RotateVector(FVector(-MinExtentX, -MinExtentY, 0))); \
		Points.Add(Origin + Rotation.RotateVector(FVector(-MinExtentX, MinExtentY, 0))); \
		Points.Add(Origin + Rotation.RotateVector(FVector(MinExtentX, MinExtentY, 0))); \
		Points.Add(Origin + Rotation.RotateVector(FVector(MinExtentX, -MinExtentY, 0))); \
		UE_VLOG_CONVEXPOLY(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, Points, N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

// SPHERE

#define N_IMPLEMENT_VLOG_SPHERE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled) { \
		UE_VLOG_WIRESPHERE(World, N_VLOG_PICKER_CHANNEL, Verbose, Origin, MinimumRadius, N_VLOG_PICKER_INNER_COLOR, TEXT("")); \
		UE_VLOG_WIRESPHERE(World, N_VLOG_PICKER_CHANNEL, Verbose, Origin, MaximumRadius, N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(World, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_SPHERE_SIMPLE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled) { \
		UE_VLOG_WIRESPHERE(World, N_VLOG_PICKER_CHANNEL, Verbose, Origin, Radius, N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(World, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_SPHERE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled) { \
		UE_VLOG_WIRESPHERE(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, Origin, MinimumRadius, N_VLOG_PICKER_INNER_COLOR, TEXT("")); \
		UE_VLOG_WIRESPHERE(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, Origin, MaximumRadius, N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_SPHERE_SIMPLE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled) { \
		UE_VLOG_WIRESPHERE(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, Origin, Radius, N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#else

#define N_IMPLEMENT_VLOG_BOX()
#define N_IMPLEMENT_VLOG_BOX_SIMPLE()
#define N_IMPLEMENT_VLOG_BOX_PROJECTION()
#define N_IMPLEMENT_VLOG_BOX_SIMPLE_PROJECTION()
#define N_IMPLEMENT_VLOG_CIRCLE()
#define N_IMPLEMENT_VLOG_CIRCLE_SIMPLE()
#define N_IMPLEMENT_VLOG_CIRCLE_PROJECTION()
#define N_IMPLEMENT_VLOG_CIRCLE_SIMPLE_PROJECTION()
#define N_IMPLEMENT_VLOG_RECTANGLE()
#define N_IMPLEMENT_VLOG_RECTANGLE_SIMPLE()
#define N_IMPLEMENT_VLOG_RECTANGLE_PROJECTION()
#define N_IMPLEMENT_VLOG_RECTANGLE_SIMPLE_PROJECTION()
#define N_IMPLEMENT_VLOG_SPHERE()
#define N_IMPLEMENT_VLOG_SPHERE_SIMPLE()
#define N_IMPLEMENT_VLOG_SPHERE_PROJECTION()
#define N_IMPLEMENT_VLOG_SPHERE_SIMPLE_PROJECTION()

#endif

/**
 * Utility methods supporting picker operations.
 */
class NEXUSPICKER_API FNPickerUtils
{
public:
	static bool bVisualLoggingEnabled;
	
	static FCollisionQueryParams DefaultTraceParams;
	
	static FVector DefaultProjection;
	static FRotator DefaultRotation;

	static FRotator BaseRotation;
	static FMatrix BaseMatrix;
};