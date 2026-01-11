// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "CollisionQueryParams.h"

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
		UE_VLOG_WIREBOX(World, LogNexusPicker, Verbose, MinimumDimensions.MoveTo(Origin), NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		UE_VLOG_WIREBOX(World, LogNexusPicker, Verbose, MaximumDimensions.MoveTo(Origin), NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		UE_VLOG_LOCATION(World, LogNexusPicker, Verbose, OutLocation, NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_BOX_SIMPLE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIREBOX(World, LogNexusPicker, Verbose, Dimensions.MoveTo(Origin), NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		UE_VLOG_LOCATION(World, LogNexusPicker, Verbose, OutLocation, NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_BOX_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIREBOX(InWorld, LogNexusPicker, Verbose, MinimumDimensions.MoveTo(Origin), NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		UE_VLOG_WIREBOX(InWorld, LogNexusPicker, Verbose, MaximumDimensions.MoveTo(Origin), NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		UE_VLOG_LOCATION(InWorld, LogNexusPicker, Verbose, OutLocation, NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_BOX_SIMPLE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIREBOX(InWorld, LogNexusPicker, Verbose, Dimensions.MoveTo(Origin), NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		UE_VLOG_LOCATION(InWorld, LogNexusPicker, Verbose, OutLocation, NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

// CIRCLE

#define N_IMPLEMENT_VLOG_CIRCLE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIRECIRCLE(World, LogNexusPicker, Verbose, Origin, Rotation.RotateVector(FVector::UpVector), MinimumRadius, NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		UE_VLOG_WIRECIRCLE(World, LogNexusPicker, Verbose, Origin, Rotation.RotateVector(FVector::UpVector), MaximumRadius, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		UE_VLOG_LOCATION(World, LogNexusPicker, Verbose, OutLocation, NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_CIRCLE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIRECIRCLE(InWorld, LogNexusPicker, Verbose, Origin, Rotation.RotateVector(FVector::UpVector), MinimumRadius, NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		UE_VLOG_WIRECIRCLE(InWorld, LogNexusPicker, Verbose, Origin, Rotation.RotateVector(FVector::UpVector), MaximumRadius, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		UE_VLOG_LOCATION(InWorld, LogNexusPicker, Verbose, OutLocation, NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocation.ToCompactString()); \
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
		UE_VLOG_SEGMENT(World, LogNexusPicker, Verbose, VerticesVariable[0], VerticesVariable[1], Color, TEXT("")); \
		UE_VLOG_SEGMENT(World, LogNexusPicker, Verbose, VerticesVariable[1], VerticesVariable[2], Color, TEXT("")); \
		UE_VLOG_SEGMENT(World, LogNexusPicker, Verbose, VerticesVariable[2], VerticesVariable[3], Color, TEXT("")); \
		UE_VLOG_SEGMENT(World, LogNexusPicker, Verbose, VerticesVariable[3], VerticesVariable[0], Color, TEXT(""));


#define N_IMPLEMENT_VLOG_RECTANGLE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		N_IMPLEMENT_VLOG_RECTANGLE_VERTICES(Origin, MinimumDimensions, Rotation, MinimumPoints) \
		N_IMPLEMENT_VLOG_RECTANGLE_DRAW(World, MinimumPoints, NEXUS::Picker::VLog::InnerColor); \
		N_IMPLEMENT_VLOG_RECTANGLE_VERTICES(Origin, MaximumDimensions, Rotation, MaximumPoints) \
		N_IMPLEMENT_VLOG_RECTANGLE_DRAW(World, MaximumPoints, NEXUS::Picker::VLog::OuterColor); \
		UE_VLOG_LOCATION(World, LogNexusPicker, Verbose, OutLocation, NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_RECTANGLE_SIMPLE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		N_IMPLEMENT_VLOG_RECTANGLE_VERTICES(Origin, Dimensions, Rotation, Points) \
		N_IMPLEMENT_VLOG_RECTANGLE_DRAW(World, Points, NEXUS::Picker::VLog::OuterColor); \
		UE_VLOG_LOCATION(World, LogNexusPicker, Verbose, OutLocation, NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_RECTANGLE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		N_IMPLEMENT_VLOG_RECTANGLE_VERTICES(Origin, MinimumDimensions, Rotation, MinimumPoints) \
		N_IMPLEMENT_VLOG_RECTANGLE_DRAW(InWorld, MinimumPoints, NEXUS::Picker::VLog::InnerColor); \
		N_IMPLEMENT_VLOG_RECTANGLE_VERTICES(Origin, MaximumDimensions, Rotation, MaximumPoints) \
		N_IMPLEMENT_VLOG_RECTANGLE_DRAW(InWorld, MaximumPoints, NEXUS::Picker::VLog::OuterColor); \
		UE_VLOG_LOCATION(InWorld, LogNexusPicker, Verbose, OutLocation, NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_RECTANGLE_SIMPLE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		N_IMPLEMENT_VLOG_RECTANGLE_VERTICES(Origin, Dimensions, Rotation, Points) \
		N_IMPLEMENT_VLOG_RECTANGLE_DRAW(InWorld, Points, NEXUS::Picker::VLog::OuterColor); \
		UE_VLOG_LOCATION(InWorld, LogNexusPicker, Verbose, OutLocation, NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

// SPHERE

#define N_IMPLEMENT_VLOG_SPHERE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIRESPHERE(World, LogNexusPicker, Verbose, Origin, MinimumRadius, NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		UE_VLOG_WIRESPHERE(World, LogNexusPicker, Verbose, Origin, MaximumRadius, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		UE_VLOG_LOCATION(World, LogNexusPicker, Verbose, OutLocation, NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_SPHERE_SIMPLE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIRESPHERE(World, LogNexusPicker, Verbose, Origin, Radius, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		UE_VLOG_LOCATION(World, LogNexusPicker, Verbose, OutLocation, NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_SPHERE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIRESPHERE(InWorld, LogNexusPicker, Verbose, Origin, MinimumRadius, NEXUS::Picker::VLog::InnerColor, TEXT("")); \
		UE_VLOG_WIRESPHERE(InWorld, LogNexusPicker, Verbose, Origin, MaximumRadius, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		UE_VLOG_LOCATION(InWorld, LogNexusPicker, Verbose, OutLocation, NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocation.ToCompactString()); \
	}

#define N_IMPLEMENT_VLOG_SPHERE_SIMPLE_PROJECTION() \
	if(InWorld != nullptr && FNPickerUtils::bVisualLoggingEnabled && FVisualLogger::IsRecording()) { \
		UE_VLOG_WIRESPHERE(InWorld, LogNexusPicker, Verbose, Origin, Radius, NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		UE_VLOG_LOCATION(InWorld, LogNexusPicker, Verbose, OutLocation, NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocation.ToCompactString()); \
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
			UE_VLOG_SEGMENT(World, LogNexusPicker, Verbose, SplinePoints[i], SplinePoints[i+1], NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		} \
		if(SplineComponent->IsClosedLoop()) \
		{ \
			UE_VLOG_SEGMENT(World, LogNexusPicker, Verbose, SplinePoints[NumSegments-1], SplinePoints[0], NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		} \
		UE_VLOG_LOCATION(World, LogNexusPicker, Verbose, OutLocation, NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocation.ToCompactString()); \
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
			UE_VLOG_SEGMENT(InWorld, LogNexusPicker, Verbose, SplinePoints[i], SplinePoints[i+1], NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		} \
		if(SplineComponent->IsClosedLoop()) \
		{ \
			UE_VLOG_SEGMENT(InWorld, LogNexusPicker, Verbose, SplinePoints[NumSegments-1], SplinePoints[0], NEXUS::Picker::VLog::OuterColor, TEXT("")); \
		} \
		UE_VLOG_LOCATION(InWorld, LogNexusPicker, Verbose, OutLocation, NEXUS::Picker::VLog::PointSize, NEXUS::Picker::VLog::PointColor, TEXT("%s"), *OutLocation.ToCompactString()); \
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