// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "CollisionQueryParams.h"

NEXUSPICKER_API DECLARE_LOG_CATEGORY_EXTERN(LogNexusPicker, Log, All);

#define N_PICKER_TOLERANCE 0.01f

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
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIREBOX(World, N_VLOG_PICKER_CHANNEL, Verbose, MinimumDimensions.MoveTo(Origin), N_VLOG_PICKER_INNER_COLOR, TEXT("")); \
		UE_VLOG_WIREBOX(World, N_VLOG_PICKER_CHANNEL, Verbose, MaximumDimensions.MoveTo(Origin), N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(World, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_BOX_SIMPLE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIREBOX(World, N_VLOG_PICKER_CHANNEL, Verbose, Dimensions.MoveTo(Origin), N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(World, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_BOX_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIREBOX(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, MinimumDimensions.MoveTo(Origin), N_VLOG_PICKER_INNER_COLOR, TEXT("")); \
		UE_VLOG_WIREBOX(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, MaximumDimensions.MoveTo(Origin), N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_BOX_SIMPLE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIREBOX(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, Dimensions.MoveTo(Origin), N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

// CIRCLE

#define N_IMPLEMENT_VLOG_CIRCLE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIRECIRCLE(World, N_VLOG_PICKER_CHANNEL, Verbose, Origin, Rotation.RotateVector(FVector::UpVector), MinimumRadius, N_VLOG_PICKER_INNER_COLOR, TEXT("")); \
		UE_VLOG_WIRECIRCLE(World, N_VLOG_PICKER_CHANNEL, Verbose, Origin, Rotation.RotateVector(FVector::UpVector), MaximumRadius, N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(World, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_CIRCLE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIRECIRCLE(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, Origin, Rotation.RotateVector(FVector::UpVector), MinimumRadius, N_VLOG_PICKER_INNER_COLOR, TEXT("")); \
		UE_VLOG_WIRECIRCLE(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, Origin, Rotation.RotateVector(FVector::UpVector), MaximumRadius, N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

// RECTANGLE

#define N_IMPLEMENT_VLOG_RECTANGLE_VERTICES(Origin, Dimensions, Rotation, VerticesVariable) \
		const float VerticesVariable##ExtentX = Dimensions.X * 0.5f; \
		const float VerticesVariable##ExtentY = Dimensions.Y * 0.5f; \
		TArray<FVector> VerticesVariable { \
			Origin + Rotation.RotateVector(FVector(-VerticesVariable##ExtentX, -VerticesVariable##ExtentY, 0)), \
			Origin + Rotation.RotateVector(FVector(-VerticesVariable##ExtentX, VerticesVariable##ExtentY, 0)), \
			Origin + Rotation.RotateVector(FVector(VerticesVariable##ExtentX, VerticesVariable##ExtentY, 0)), \
			Origin + Rotation.RotateVector(FVector(VerticesVariable##ExtentX, -VerticesVariable##ExtentY, 0)) \
		};

// Not a superfan of using segments
#define N_IMPLEMENT_VLOG_RECTANGLE_DRAW(World, VerticesVariable, Color) \
		UE_VLOG_SEGMENT(World, N_VLOG_PICKER_CHANNEL, Verbose, VerticesVariable[0], VerticesVariable[1], Color, TEXT("")); \
		UE_VLOG_SEGMENT(World, N_VLOG_PICKER_CHANNEL, Verbose, VerticesVariable[1], VerticesVariable[2], Color, TEXT("")); \
		UE_VLOG_SEGMENT(World, N_VLOG_PICKER_CHANNEL, Verbose, VerticesVariable[2], VerticesVariable[3], Color, TEXT("")); \
		UE_VLOG_SEGMENT(World, N_VLOG_PICKER_CHANNEL, Verbose, VerticesVariable[3], VerticesVariable[0], Color, TEXT(""));


#define N_IMPLEMENT_VLOG_RECTANGLE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		N_IMPLEMENT_VLOG_RECTANGLE_VERTICES(Origin, MinimumDimensions, Rotation, MinimumPoints) \
		N_IMPLEMENT_VLOG_RECTANGLE_DRAW(World, MinimumPoints, N_VLOG_PICKER_INNER_COLOR); \
		N_IMPLEMENT_VLOG_RECTANGLE_VERTICES(Origin, MaximumDimensions, Rotation, MaximumPoints) \
		N_IMPLEMENT_VLOG_RECTANGLE_DRAW(World, MaximumPoints, N_VLOG_PICKER_OUTER_COLOR); \
		UE_VLOG_LOCATION(World, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_RECTANGLE_SIMPLE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		N_IMPLEMENT_VLOG_RECTANGLE_VERTICES(Origin, Dimensions, Rotation, Points) \
		N_IMPLEMENT_VLOG_RECTANGLE_DRAW(World, Points, N_VLOG_PICKER_OUTER_COLOR); \
		UE_VLOG_LOCATION(World, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_RECTANGLE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		N_IMPLEMENT_VLOG_RECTANGLE_VERTICES(Origin, MinimumDimensions, Rotation, MinimumPoints) \
		N_IMPLEMENT_VLOG_RECTANGLE_DRAW(InWorld, MinimumPoints, N_VLOG_PICKER_INNER_COLOR); \
		N_IMPLEMENT_VLOG_RECTANGLE_VERTICES(Origin, MaximumDimensions, Rotation, MaximumPoints) \
		N_IMPLEMENT_VLOG_RECTANGLE_DRAW(InWorld, MaximumPoints, N_VLOG_PICKER_OUTER_COLOR); \
		UE_VLOG_LOCATION(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_RECTANGLE_SIMPLE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		N_IMPLEMENT_VLOG_RECTANGLE_VERTICES(Origin, Dimensions, Rotation, Points) \
		N_IMPLEMENT_VLOG_RECTANGLE_DRAW(InWorld, Points, N_VLOG_PICKER_OUTER_COLOR); \
		UE_VLOG_LOCATION(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

// SPHERE

#define N_IMPLEMENT_VLOG_SPHERE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIRESPHERE(World, N_VLOG_PICKER_CHANNEL, Verbose, Origin, MinimumRadius, N_VLOG_PICKER_INNER_COLOR, TEXT("")); \
		UE_VLOG_WIRESPHERE(World, N_VLOG_PICKER_CHANNEL, Verbose, Origin, MaximumRadius, N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(World, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_SPHERE_SIMPLE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIRESPHERE(World, N_VLOG_PICKER_CHANNEL, Verbose, Origin, Radius, N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(World, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_SPHERE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIRESPHERE(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, Origin, MinimumRadius, N_VLOG_PICKER_INNER_COLOR, TEXT("")); \
		UE_VLOG_WIRESPHERE(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, Origin, MaximumRadius, N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_SPHERE_SIMPLE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIRESPHERE(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, Origin, Radius, N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_SPLINE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		TArray<FVector> SplinePoints; \
		const float SplineLength = SplineComponent->GetSplineLength(); \
		const int32 NumSegments = FMath::Max(20, FMath::RoundToInt(SplineLength / 20.0f)); \
		const float DistancePerSegment = SplineLength / NumSegments; \
		SplinePoints.Reserve(NumSegments); \
		for (int32 i = 0; i <= NumSegments; ++i) \
		{ \
			const float Distance = DistancePerSegment * i; \
			const FVector Point = SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World); \
			SplinePoints.Add(Point); \
		} \
		for (int32 i = 0; i < NumSegments; ++i) \
		{ \
			UE_VLOG_SEGMENT(World, N_VLOG_PICKER_CHANNEL, Verbose, SplinePoints[i], SplinePoints[i+1], N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		} \
		if(SplineComponent->IsClosedLoop()) \
		{ \
			UE_VLOG_SEGMENT(World, N_VLOG_PICKER_CHANNEL, Verbose, SplinePoints[NumSegments-1], SplinePoints[0], N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		} \
		UE_VLOG_LOCATION(World, N_VLOG_PICKER_CHANNEL, Verbose, OutLocation, N_VLOG_PICKER_POINT_SIZE, N_VLOG_PICKER_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_SPLINE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		TArray<FVector> SplinePoints; \
		const float SplineLength = SplineComponent->GetSplineLength(); \
		const int32 NumSegments = FMath::Max(20, FMath::RoundToInt(SplineLength / 20.0f)); \
		const float DistancePerSegment = SplineLength / NumSegments; \
		SplinePoints.Reserve(NumSegments); \
		for (int32 i = 0; i <= NumSegments; ++i) \
		{ \
			const float Distance = DistancePerSegment * i; \
			const FVector Point = SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World); \
			SplinePoints.Add(Point); \
		} \
		for (int32 i = 0; i < NumSegments; ++i) \
		{ \
			UE_VLOG_SEGMENT(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, SplinePoints[i], SplinePoints[i+1], N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		} \
		if(SplineComponent->IsClosedLoop()) \
		{ \
			UE_VLOG_SEGMENT(InWorld, N_VLOG_PICKER_CHANNEL, Verbose, SplinePoints[NumSegments-1], SplinePoints[0], N_VLOG_PICKER_OUTER_COLOR, TEXT("")); \
		} \
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
#define N_IMPLEMENT_VLOG_SPLINE()
#define N_IMPLEMENT_VLOG_SPLINE_PROJECTION()

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