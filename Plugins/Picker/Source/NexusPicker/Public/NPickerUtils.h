// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "CollisionQueryParams.h"

// ReSharper disable CppUnusedIncludeDirective
#include "NCoreMinimal.h"
#include "NPickerSubsystem.h"
// ReSharper restore CppUnusedIncludeDirective

#define N_PICKER_TOLERANCE 0.001f

#define N_VLOG_INNER_COLOR FColor::White
#define N_VLOG_OUTER_COLOR FColor::Black
#define N_VLOG_POINT_COLOR FColor::Green
#define N_VLOG_POINT_SIZE 2.f


#define N_VARIABLES_PICKER_PROJECTION() \
	const UWorld* InWorld = nullptr, const FVector& Projection = FNPickerUtils::DefaultProjection, const ECollisionChannel CollisionChannel = ECollisionChannel::ECC_WorldStatic

#define N_IMPLEMENT_PICKER_PROJECTION() \
	FHitResult HitResult(ForceInit); \
	if (InWorld->LineTraceSingleByChannel(HitResult, OutLocation, (OutLocation + Projection), CollisionChannel, FNPickerUtils::DefaultTraceParams)) \
	{ \
		OutLocation = HitResult.Location; \
	}

#if ENABLE_VISUAL_LOG

// SPHERE

#define N_IMPLEMENT_VLOG_SPHERE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr) { \
		const UObject* LogOwner = UNPickerSubsystem::Get(World); \
		UE_VLOG_SPHERE(LogOwner, LogNexus, Verbose, Origin, MinimumRadius, N_VLOG_INNER_COLOR, TEXT("")); \
		UE_VLOG_SPHERE(LogOwner, LogNexus, Verbose, Origin, MaximumRadius, N_VLOG_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(LogOwner, LogNexus, Verbose, OutLocation, N_VLOG_POINT_SIZE, N_VLOG_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	} \

#define N_IMPLEMENT_VLOG_SPHERE_SIMPLE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr) { \
		const UObject* LogOwner = UNPickerSubsystem::Get(World); \
		UE_VLOG_SPHERE(LogOwner, LogNexus, Verbose, Origin, Radius, N_VLOG_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(LogOwner, LogNexus, Verbose, OutLocation, N_VLOG_POINT_SIZE, N_VLOG_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	} \

#define N_IMPLEMENT_VLOG_SPHERE_PROJECTION() \
	if(InWorld != nullptr) { \
		const UObject* LogOwner = UNPickerSubsystem::Get(InWorld); \
		UE_VLOG_SPHERE(LogOwner, LogNexus, Verbose, Origin, MinimumRadius, N_VLOG_INNER_COLOR, TEXT("")); \
		UE_VLOG_SPHERE(LogOwner, LogNexus, Verbose, Origin, MaximumRadius, N_VLOG_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(LogOwner, LogNexus, Verbose, OutLocation, N_VLOG_POINT_SIZE, N_VLOG_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	} \

#define N_IMPLEMENT_VLOG_SPHERE_SIMPLE_PROJECTION() \
	if(InWorld != nullptr) { \
		const UObject* LogOwner = UNPickerSubsystem::Get(InWorld); \
		UE_VLOG_SPHERE(LogOwner, LogNexus, Verbose, Origin, Radius, N_VLOG_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(LogOwner, LogNexus, Verbose, OutLocation, N_VLOG_POINT_SIZE, N_VLOG_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	} \

// BOX

#define N_IMPLEMENT_VLOG_BOX() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr) { \
		const UObject* LogOwner = UNPickerSubsystem::Get(World); \
		UE_VLOG_BOX(LogOwner, LogNexus, Verbose, MinimumDimensions.MoveTo(Origin), N_VLOG_INNER_COLOR, TEXT("")); \
		UE_VLOG_BOX(LogOwner, LogNexus, Verbose, MaximumDimensions.MoveTo(Origin), N_VLOG_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(LogOwner, LogNexus, Verbose, OutLocation, N_VLOG_POINT_SIZE, N_VLOG_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	} \

#define N_IMPLEMENT_VLOG_BOX_SIMPLE() \
	const UWorld* World = GEngine->GetCurrentPlayWorld(); \
	if(World != nullptr) { \
		const UObject* LogOwner = UNPickerSubsystem::Get(World); \
		UE_VLOG_BOX(LogOwner, LogNexus, Verbose, Dimensions.MoveTo(Origin), N_VLOG_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(LogOwner, LogNexus, Verbose, OutLocation, N_VLOG_POINT_SIZE, N_VLOG_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	} \

#define N_IMPLEMENT_VLOG_BOX_PROJECTION() \
	if(InWorld != nullptr) { \
		const UObject* LogOwner = UNPickerSubsystem::Get(InWorld); \
		UE_VLOG_BOX(LogOwner, LogNexus, Verbose, MinimumDimensions.MoveTo(Origin), N_VLOG_INNER_COLOR, TEXT("")); \
		UE_VLOG_BOX(LogOwner, LogNexus, Verbose, MaximumDimensions.MoveTo(Origin), N_VLOG_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(LogOwner, LogNexus, Verbose, OutLocation, N_VLOG_POINT_SIZE, N_VLOG_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	} \

#define N_IMPLEMENT_VLOG_BOX_SIMPLE_PROJECTION() \
	if(InWorld != nullptr) { \
		const UObject* LogOwner = UNPickerSubsystem::Get(InWorld); \
		UE_VLOG_BOX(LogOwner, LogNexus, Verbose, Dimensions.MoveTo(Origin), N_VLOG_OUTER_COLOR, TEXT("")); \
		UE_VLOG_LOCATION(LogOwner, LogNexus, Verbose, OutLocation, N_VLOG_POINT_SIZE, N_VLOG_POINT_COLOR, TEXT("%s"), *OutLocation.ToCompactString()); \
	} \

#else

#define N_IMPLEMENT_VLOG_SPHERE()
#define N_IMPLEMENT_VLOG_SPHERE_SIMPLE()
#define N_IMPLEMENT_VLOG_SPHERE_PROJECTION()
#define N_IMPLEMENT_VLOG_SPHERE_SIMPLE_PROJECTION()
#define N_IMPLEMENT_VLOG_BOX()
#define N_IMPLEMENT_VLOG_BOX_SIMPLE()
#define N_IMPLEMENT_VLOG_BOX_PROJECTION()
#define N_IMPLEMENT_VLOG_BOX_SIMPLE_PROJECTION()

#endif

/**
 * Utility methods supporting picker operations.
 */
class NEXUSPICKER_API FNPickerUtils
{
public:
	static FCollisionQueryParams DefaultTraceParams;
	
	static FVector DefaultProjection;
	static FRotator DefaultRotation;

	static FRotator BaseRotation;
	static FMatrix BaseMatrix;
};