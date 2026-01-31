// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include "NPickerMinimal.h"
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
#define N_IMPLEMENT_PICKER_PROJECTION_TRACE_PREFIX \
	FHitResult HitResult(ForceInit);
#define N_IMPLEMENT_PICKER_PROJECTION_TRACE \
	if (Params.CachedWorld->LineTraceSingleByChannel(HitResult, Location, (Location + Params.Projection), Params.CollisionChannel, FNPickerUtils::DefaultTraceParams)) \
	{ \
		Location = HitResult.Location; \
	}

#define N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1_PREFIX \
	UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Params.CachedWorld); \
	FNavLocation NavLocation;
#define N_IMPLEMENT_PICKER_PROJECTION_NAVMESH_V1 \
	NavigationSystem->ProjectPointToNavigation(Location, NavLocation); \
	if (Location != NavLocation.Location) \
	{ \
		Location = NavLocation.Location; \
	}

#if ENABLE_VISUAL_LOG

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