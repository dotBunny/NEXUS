// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCollisionVisualizerSettings.h"


class FNCollisionVisualizerUtils
{
public:
	
	static void DoLineTraceSingle(const FNCollisionVisualizerSettings& Settings, 
		const UWorld* World, const FVector& StartPosition, const FVector& EndPosition);
	
	static void DoLineTraceMulti(const FNCollisionVisualizerSettings& Settings, 
		const UWorld* World, const FVector& StartPosition, const FVector& EndPosition);
	
	static void DoLineTraceTest(const FNCollisionVisualizerSettings& Settings,
		const UWorld* World, const FVector& StartPosition, const FVector& EndPosition);
	
	static void DoSweepSingle(const FNCollisionVisualizerSettings& Settings, 
		const UWorld* World, const FVector& StartPosition, const FVector& EndPosition, const FQuat& Rotation);
	
	static void DoSweepMulti(const FNCollisionVisualizerSettings& Settings, 
		const UWorld* World, const FVector& StartPosition, const FVector& EndPosition, const FQuat& Rotation);
	
	static void DoSweepTest(const FNCollisionVisualizerSettings& Settings, 
		const UWorld* World, const FVector& StartPosition, const FVector& EndPosition, const FQuat& Rotation);
	
	static void DoOverlapBlocking(const FNCollisionVisualizerSettings& Settings, 
		const UWorld* World, const FVector& Position, const FQuat& Rotation);
	
	static void DoOverlapAny(const FNCollisionVisualizerSettings& Settings, 
		const UWorld* World, const FVector& Position, const FQuat& Rotation);
	
	static void DoOverlapMulti(const FNCollisionVisualizerSettings& Settings, 
		const UWorld* World, const FVector& Position, const FQuat& Rotation);
	
private:
	static EQueryMobilityType ToQueryMobilityType(const ECollisionVisualizerMobility Mobility)
	{
		switch (Mobility)
		{
			using enum ECollisionVisualizerMobility;
		case Any: 
			return EQueryMobilityType::Any;
		case Static: 
			return EQueryMobilityType::Static;
		case Dynamic: 
			return EQueryMobilityType::Dynamic;
		default: 
			return EQueryMobilityType::Any;
		}
	}
};
