// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "CollisionVisualizer/NCollisionVisualizerUtils.h"
#include "Engine/OverlapResult.h"
#include "NDrawDebugHelpers.h"

#define N_COLLISION_VISUALIZER_COMMON \
	bool bHit = false; 

#define N_COLLISION_VISUALIZER_OBJECT_PARAMS FCollisionObjectQueryParams(UEngineTypes::ConvertToCollisionChannel(Settings.Query.ObjectType))
#define N_COLLISION_VISUALIZER_HIT_COLOR Settings.Drawing.DrawHitColor
#define N_COLLISION_VISUALIZER_MID_COLOR Settings.Drawing.DrawMidColor
#define N_COLLISION_VISUALIZER_MISS_COLOR Settings.Drawing.DrawMissColor

void FNCollisionVisualizerUtils::DoLineTraceSingle(const FNCollisionVisualizerSettings& Settings, 
	const UWorld* World, const FVector& StartPosition, const FVector& EndPosition)
{
	N_COLLISION_VISUALIZER_COMMON
	FHitResult HitResult;
	
	
	if (Settings.Query.QueryBy == ECollisionVisualizerBy::Channel)
	{
		bHit = World->LineTraceSingleByChannel(HitResult, StartPosition, EndPosition, 
			Settings.Query.Channel, Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::ObjectType)
	{
		bHit = World->LineTraceSingleByObjectType(HitResult, StartPosition, EndPosition, N_COLLISION_VISUALIZER_OBJECT_PARAMS);
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::Profile)
	{
		bHit = World->LineTraceSingleByProfile(HitResult, StartPosition, EndPosition, 
			Settings.Query.CollisionProfileName, Settings.Options.GetCollisionQueryParams());
	}
		
#if UE_ENABLE_DEBUG_DRAWING
	if (bHit)
	{
		DrawDebugLine(World, StartPosition, HitResult.Location, N_COLLISION_VISUALIZER_HIT_COLOR, 
			false, Settings.Drawing.DrawTimer, SDPG_World, Settings.Drawing.DrawLineThickness);
		DrawDebugPoint(World, HitResult.ImpactPoint, Settings.Drawing.DrawPointSize, N_COLLISION_VISUALIZER_HIT_COLOR, 
			false, Settings.Drawing.DrawTimer, SDPG_World);
	}
	else
	{
		DrawDebugLine(World, StartPosition, EndPosition, N_COLLISION_VISUALIZER_MISS_COLOR, 
			false, Settings.Drawing.DrawTimer, SDPG_World, Settings.Drawing.DrawLineThickness);
	}
#endif // UE_ENABLE_DEBUG_DRAWING
}

void FNCollisionVisualizerUtils::DoLineTraceMulti(const FNCollisionVisualizerSettings& Settings, 
	const UWorld* World, const FVector& StartPosition, const FVector& EndPosition)
{
	N_COLLISION_VISUALIZER_COMMON
	TArray<FHitResult> HitResults;
	
	if (Settings.Query.QueryBy == ECollisionVisualizerBy::Channel)
	{
		bHit = World->LineTraceMultiByChannel(HitResults, StartPosition, EndPosition, 
			Settings.Query.Channel, Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::ObjectType)
	{
		bHit = World->LineTraceMultiByObjectType(HitResults, StartPosition, EndPosition, 
			N_COLLISION_VISUALIZER_OBJECT_PARAMS, Settings.Options.GetCollisionQueryParams());
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::Profile)
	{
		bHit = World->LineTraceMultiByProfile(HitResults, StartPosition, EndPosition, 
			Settings.Query.CollisionProfileName, Settings.Options.GetCollisionQueryParams());
	}
		
#if UE_ENABLE_DEBUG_DRAWING
	if (bHit)
	{
		// Adjust the end point to being the last hit point, unless it's by ObjectType.
		FVector TraceEndPosition = HitResults.Last().Location;
		if (Settings.Query.QueryBy == ECollisionVisualizerBy::ObjectType)
		{
			TraceEndPosition = EndPosition;
		}
		DrawDebugLine(World, StartPosition, TraceEndPosition, N_COLLISION_VISUALIZER_HIT_COLOR, 
			false, Settings.Drawing.DrawTimer, SDPG_World, Settings.Drawing.DrawLineThickness);
	}
	else
	{
		DrawDebugLine(World, StartPosition, EndPosition, 
			HitResults.Num() > 0 ? N_COLLISION_VISUALIZER_MID_COLOR : N_COLLISION_VISUALIZER_MISS_COLOR, 
			false, Settings.Drawing.DrawTimer, SDPG_World, Settings.Drawing.DrawLineThickness);
	}
	for (const FHitResult& HitResult : HitResults)
	{
		DrawDebugPoint(World, HitResult.ImpactPoint, Settings.Drawing.DrawPointSize, 
			HitResult.bBlockingHit ? N_COLLISION_VISUALIZER_HIT_COLOR : N_COLLISION_VISUALIZER_MID_COLOR, 
			false, Settings.Drawing.DrawTimer, SDPG_World);
	}
#endif // UE_ENABLE_DEBUG_DRAWING
}

void FNCollisionVisualizerUtils::DoLineTraceTest(const FNCollisionVisualizerSettings& Settings, 
                                                const UWorld* World, const FVector& StartPosition, const FVector& EndPosition)
{
	N_COLLISION_VISUALIZER_COMMON
	
	if (Settings.Query.QueryBy == ECollisionVisualizerBy::Channel)
	{
		bHit = World->LineTraceTestByChannel(StartPosition, EndPosition, 
			Settings.Query.Channel, Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::ObjectType)
	{
		bHit = World->LineTraceTestByObjectType(StartPosition, EndPosition, N_COLLISION_VISUALIZER_OBJECT_PARAMS, 
			Settings.Options.GetCollisionQueryParams());
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::Profile)
	{
		bHit = World->LineTraceTestByProfile(StartPosition, EndPosition, Settings.Query.CollisionProfileName, Settings.Options.GetCollisionQueryParams());
	}

#if UE_ENABLE_DEBUG_DRAWING
	DrawDebugLine(World, StartPosition, EndPosition, 
		bHit ? N_COLLISION_VISUALIZER_HIT_COLOR : N_COLLISION_VISUALIZER_MISS_COLOR, 
		false, Settings.Drawing.DrawTimer, SDPG_World, Settings.Drawing.DrawLineThickness);
#endif // UE_ENABLE_DEBUG_DRAWING
}



void FNCollisionVisualizerUtils::DoSweepSingle(const FNCollisionVisualizerSettings& Settings, const UWorld* World, 
	const FVector& StartPosition, const FVector& EndPosition, const FQuat& Rotation)
{
	N_COLLISION_VISUALIZER_COMMON
	const FCollisionShape CollisionShape = Settings.Query.GetCollisionShape();
	FHitResult HitResult;
	
	if (Settings.Query.QueryBy == ECollisionVisualizerBy::Channel)
	{
		bHit = World->SweepSingleByChannel(HitResult, StartPosition, EndPosition, Rotation, Settings.Query.Channel, 
			CollisionShape, Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::ObjectType)
	{
		bHit = World->SweepSingleByObjectType(HitResult, StartPosition, EndPosition, Rotation, 
			N_COLLISION_VISUALIZER_OBJECT_PARAMS, CollisionShape, Settings.Options.GetCollisionQueryParams());
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::Profile)
	{
		bHit = World->SweepSingleByProfile(HitResult, StartPosition, EndPosition, Rotation, Settings.Query.CollisionProfileName, 
			CollisionShape, Settings.Options.GetCollisionQueryParams());
	}

#if UE_ENABLE_DEBUG_DRAWING
	if (bHit)
	{
		DrawDebugPoint(World, HitResult.ImpactPoint, Settings.Drawing.DrawPointSize, N_COLLISION_VISUALIZER_HIT_COLOR, 
			false, Settings.Drawing.DrawTimer, SDPG_World);
		FNDrawDebugHelpers::DrawSweep(World, StartPosition, HitResult.Location, Rotation, CollisionShape,
			N_COLLISION_VISUALIZER_HIT_COLOR, false, Settings.Drawing.DrawTimer, SDPG_World, 
			Settings.Drawing.DrawLineThickness);
	}
	else
	{
		FNDrawDebugHelpers::DrawSweep(World, StartPosition, EndPosition, Rotation, CollisionShape, 
			N_COLLISION_VISUALIZER_MISS_COLOR, false, Settings.Drawing.DrawTimer, SDPG_World, 
			Settings.Drawing.DrawLineThickness);
	}
#endif // UE_ENABLE_DEBUG_DRAWING
}

void FNCollisionVisualizerUtils::DoSweepMulti(const FNCollisionVisualizerSettings& Settings, const UWorld* World, 
	const FVector& StartPosition, const FVector& EndPosition, const FQuat& Rotation)
{
	N_COLLISION_VISUALIZER_COMMON
	const FCollisionShape CollisionShape = Settings.Query.GetCollisionShape();
	TArray<FHitResult> HitResults;
	
	if (Settings.Query.QueryBy == ECollisionVisualizerBy::Channel)
	{
		bHit = World->SweepMultiByChannel(HitResults, StartPosition, EndPosition, Rotation, Settings.Query.Channel, 
			CollisionShape, Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::ObjectType)
	{
		bHit = World->SweepMultiByObjectType(HitResults, StartPosition, EndPosition, Rotation, 
			N_COLLISION_VISUALIZER_OBJECT_PARAMS, CollisionShape, Settings.Options.GetCollisionQueryParams());
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::Profile)
	{
		bHit = World->SweepMultiByProfile(HitResults, StartPosition, EndPosition, Rotation, Settings.Query.CollisionProfileName, 
			CollisionShape, Settings.Options.GetCollisionQueryParams());
	}

#if UE_ENABLE_DEBUG_DRAWING
	if (bHit)
	{
		FVector SweepEnd = HitResults.Last().Location;
		if (Settings.Query.QueryBy == ECollisionVisualizerBy::ObjectType)
		{
			SweepEnd = EndPosition;
		}
		else if (HitResults.Last().bStartPenetrating)
		{
			SweepEnd = EndPosition;
		}

		FNDrawDebugHelpers::DrawSweep(World, StartPosition, SweepEnd, Rotation, CollisionShape, 
			N_COLLISION_VISUALIZER_HIT_COLOR, false, Settings.Drawing.DrawTimer, SDPG_World, 
			Settings.Drawing.DrawLineThickness);
	}
	else
	{
		FNDrawDebugHelpers::DrawSweep(World, StartPosition, EndPosition, Rotation, CollisionShape, 
			HitResults.Num() > 0 ? N_COLLISION_VISUALIZER_MID_COLOR : N_COLLISION_VISUALIZER_MISS_COLOR, 
			false, Settings.Drawing.DrawTimer, SDPG_World, Settings.Drawing.DrawLineThickness);
	}

	for (const FHitResult& HitResult : HitResults)
	{
		DrawDebugPoint(World, HitResult.ImpactPoint, Settings.Drawing.DrawPointSize, 
			HitResult.bBlockingHit ? N_COLLISION_VISUALIZER_HIT_COLOR : N_COLLISION_VISUALIZER_MID_COLOR, 
			false, Settings.Drawing.DrawTimer, SDPG_World);
	}
#endif // UE_ENABLE_DEBUG_DRAWING
}

void FNCollisionVisualizerUtils::DoSweepTest(const FNCollisionVisualizerSettings& Settings, const UWorld* World, const FVector& StartPosition, const FVector& EndPosition, const FQuat& Rotation)
{
	N_COLLISION_VISUALIZER_COMMON
	const FCollisionShape& CollisionShape = Settings.Query.GetCollisionShape();
	
	if (Settings.Query.QueryBy == ECollisionVisualizerBy::Channel)
	{
		bHit = World->SweepTestByChannel(StartPosition, EndPosition, Rotation, Settings.Query.Channel, CollisionShape, 
			Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::ObjectType)
	{
		bHit = World->SweepTestByObjectType(StartPosition, EndPosition, Rotation, N_COLLISION_VISUALIZER_OBJECT_PARAMS, 
			CollisionShape, Settings.Options.GetCollisionQueryParams());
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::Profile)
	{
		bHit = World->SweepTestByProfile(StartPosition, EndPosition, Rotation, Settings.Query.CollisionProfileName, 
			CollisionShape, Settings.Options.GetCollisionQueryParams());
	}

#if UE_ENABLE_DEBUG_DRAWING
	FNDrawDebugHelpers::DrawSweep(World, StartPosition, EndPosition, Rotation, CollisionShape, 
		bHit ? N_COLLISION_VISUALIZER_HIT_COLOR: N_COLLISION_VISUALIZER_MISS_COLOR, false, 
		Settings.Drawing.DrawTimer, SDPG_World, Settings.Drawing.DrawLineThickness);
#endif // UE_ENABLE_DEBUG_DRAWING
}

void FNCollisionVisualizerUtils::DoOverlapBlocking(const FNCollisionVisualizerSettings& Settings, const UWorld* World,
	const FVector& Position, const FQuat& Rotation)
{
	N_COLLISION_VISUALIZER_COMMON
	const FCollisionShape& CollisionShape = Settings.Query.GetCollisionShape();
	
	if (Settings.Query.QueryBy == ECollisionVisualizerBy::Channel)
	{
		bHit = World->OverlapBlockingTestByChannel(Position, Rotation, Settings.Query.Channel, CollisionShape, 
			Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::ObjectType)
	{
		bHit = World->OverlapAnyTestByObjectType(Position, Rotation, N_COLLISION_VISUALIZER_OBJECT_PARAMS, 
			CollisionShape, Settings.Options.GetCollisionQueryParams());
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::Profile)
	{
		bHit = World->OverlapBlockingTestByProfile(Position, Rotation, Settings.Query.CollisionProfileName, CollisionShape, 
			Settings.Options.GetCollisionQueryParams());
	}

#if UE_ENABLE_DEBUG_DRAWING
	FNDrawDebugHelpers::DrawCollisionShape(World, Position, Rotation, CollisionShape, 
		bHit ? N_COLLISION_VISUALIZER_HIT_COLOR : N_COLLISION_VISUALIZER_MISS_COLOR, false, Settings.Drawing.DrawTimer, 
		SDPG_World, Settings.Drawing.DrawLineThickness);
#endif // UE_ENABLE_DEBUG_DRAWING
}

void FNCollisionVisualizerUtils::DoOverlapAny(const FNCollisionVisualizerSettings& Settings, 
	const UWorld* World, const FVector& Position, const FQuat& Rotation)
{
	N_COLLISION_VISUALIZER_COMMON
	const FCollisionShape& CollisionShape = Settings.Query.GetCollisionShape();
	
	if (Settings.Query.QueryBy == ECollisionVisualizerBy::Channel)
	{
		bHit = World->OverlapAnyTestByChannel(Position, Rotation, Settings.Query.Channel, CollisionShape, 
			Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::ObjectType)
	{
		bHit = World->OverlapAnyTestByObjectType(Position, Rotation, N_COLLISION_VISUALIZER_OBJECT_PARAMS, 
			CollisionShape, Settings.Options.GetCollisionQueryParams());
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::Profile)
	{
		bHit = World->OverlapAnyTestByProfile(Position, Rotation, Settings.Query.CollisionProfileName, CollisionShape, 
			Settings.Options.GetCollisionQueryParams());
	}

#if UE_ENABLE_DEBUG_DRAWING
	FNDrawDebugHelpers::DrawCollisionShape(World, Position, Rotation, CollisionShape, 
		bHit ? N_COLLISION_VISUALIZER_HIT_COLOR : N_COLLISION_VISUALIZER_MISS_COLOR, false, Settings.Drawing.DrawTimer, 
		SDPG_World, Settings.Drawing.DrawLineThickness);
#endif // UE_ENABLE_DEBUG_DRAWING
}

void FNCollisionVisualizerUtils::DoOverlapMulti(const FNCollisionVisualizerSettings& Settings, 
	const UWorld* World, const FVector& Position, const FQuat& Rotation)
{
	N_COLLISION_VISUALIZER_COMMON
	TArray<FOverlapResult>OverlapsResults;
	const FCollisionShape& CollisionShape = Settings.Query.GetCollisionShape();
	
	if (Settings.Query.QueryBy == ECollisionVisualizerBy::Channel)
	{
		bHit = World->OverlapMultiByChannel(OverlapsResults, Position, Rotation, Settings.Query.Channel, CollisionShape, 
			Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::ObjectType)
	{
		bHit = World->OverlapMultiByObjectType(OverlapsResults, Position, Rotation, 
			N_COLLISION_VISUALIZER_OBJECT_PARAMS, CollisionShape, Settings.Options.GetCollisionQueryParams());
	}
	else if (Settings.Query.QueryBy == ECollisionVisualizerBy::Profile)
	{
		bHit = World->OverlapMultiByProfile(OverlapsResults, Position, Rotation, Settings.Query.CollisionProfileName, 
			CollisionShape, Settings.Options.GetCollisionQueryParams());
	}

#if UE_ENABLE_DEBUG_DRAWING
	FNDrawDebugHelpers::DrawCollisionShape(World, Position, Rotation, CollisionShape, 
		bHit ? N_COLLISION_VISUALIZER_HIT_COLOR : N_COLLISION_VISUALIZER_MISS_COLOR, false, Settings.Drawing.DrawTimer, 
		SDPG_World, Settings.Drawing.DrawLineThickness);
#endif // UE_ENABLE_DEBUG_DRAWING
}