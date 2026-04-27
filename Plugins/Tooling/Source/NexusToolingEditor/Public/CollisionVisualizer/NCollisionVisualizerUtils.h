// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCollisionVisualizerSettings.h"


/**
 * Query dispatch and debug-draw helpers for the collision visualizer. Each Do* entry issues the
 * corresponding UE collision query against World using Settings and draws the result into the
 * viewport using the configured colors/thickness/lifetime.
 */
class FNCollisionVisualizerUtils
{
public:

	/** Issue a single-result line trace and draw the outcome. */
	static void DoLineTraceSingle(const FNCollisionVisualizerSettings& Settings,
		const UWorld* World, const FVector& StartPosition, const FVector& EndPosition);

	/** Issue a multi-result line trace and draw each hit. */
	static void DoLineTraceMulti(const FNCollisionVisualizerSettings& Settings,
		const UWorld* World, const FVector& StartPosition, const FVector& EndPosition);

	/** Issue a boolean line-trace test and draw a hit/miss segment. */
	static void DoLineTraceTest(const FNCollisionVisualizerSettings& Settings,
		const UWorld* World, const FVector& StartPosition, const FVector& EndPosition);

	/** Issue a single-result shape sweep and draw the outcome. */
	static void DoSweepSingle(const FNCollisionVisualizerSettings& Settings,
		const UWorld* World, const FVector& StartPosition, const FVector& EndPosition, const FQuat& Rotation);

	/** Issue a multi-result shape sweep and draw each hit. */
	static void DoSweepMulti(const FNCollisionVisualizerSettings& Settings,
		const UWorld* World, const FVector& StartPosition, const FVector& EndPosition, const FQuat& Rotation);

	/** Issue a boolean shape-sweep test and draw a hit/miss result. */
	static void DoSweepTest(const FNCollisionVisualizerSettings& Settings,
		const UWorld* World, const FVector& StartPosition, const FVector& EndPosition, const FQuat& Rotation);

	/** Issue a blocking-only overlap query at Position/Rotation and draw the outcome. */
	static void DoOverlapBlocking(const FNCollisionVisualizerSettings& Settings,
		const UWorld* World, const FVector& Position, const FQuat& Rotation);

	/** Issue an any-overlap query at Position/Rotation and draw the outcome. */
	static void DoOverlapAny(const FNCollisionVisualizerSettings& Settings,
		const UWorld* World, const FVector& Position, const FQuat& Rotation);

	/** Issue a multi-overlap query at Position/Rotation and draw each overlapping primitive. */
	static void DoOverlapMulti(const FNCollisionVisualizerSettings& Settings,
		const UWorld* World, const FVector& Position, const FQuat& Rotation);

private:
	/** Map the visualizer's mobility enum onto the engine's EQueryMobilityType. */
	static EQueryMobilityType ToQueryMobilityType(const ENCollisionVisualizerMobility Mobility)
	{
		switch (Mobility)
		{
			using enum ENCollisionVisualizerMobility;
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
