// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "CollisionQueryTest/NCollisionQueryTestUtils.h"
#include "Engine/OverlapResult.h"
#include "NDrawDebugHelpers.h"

#define N_COLLISION_QUERY_TEST_COMMON \
	bool bHit = false; \

#define N_COLLISION_QUERY_OBJECT_PARAMS FCollisionObjectQueryParams(UEngineTypes::ConvertToCollisionChannel(Settings.Query.ObjectType))
#define N_COLLISION_QUERY_HIT_COLOR FColor::Green
#define N_COLLISION_QUERY_MID_COLOR FColor::Blue
#define N_COLLISION_QUERY_MISS_COLOR FColor::Red
#define N_COLLISION_QUERY_NO_SIMULATION_COLOR FColor::Yellow

// TODO: Detect if world is playing and use the NO_SIM

void FNCollisionQueryTestUtils::DoLineTraceSingle(const FNCollisionQueryTestSettings& Settings, 
	const UWorld* World, const FVector& StartPosition, const FVector& EndPosition)
{
	N_COLLISION_QUERY_TEST_COMMON
	FHitResult HitResult;
	
	
	if (Settings.Query.QueryBy == ECollisionQueryTestBy::Channel)
	{
		bHit = World->LineTraceSingleByChannel(HitResult, StartPosition, EndPosition, 
			Settings.Query.Channel, Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::ObjectType)
	{
		bHit = World->LineTraceSingleByObjectType(HitResult, StartPosition, EndPosition, N_COLLISION_QUERY_OBJECT_PARAMS);
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::Profile)
	{
		bHit = World->LineTraceSingleByProfile(HitResult, StartPosition, EndPosition, 
			Settings.Query.CollisionProfileName, Settings.Options.GetCollisionQueryParams());
	}
		
#if UE_ENABLE_DEBUG_DRAWING
	if (bHit)
	{
		DrawDebugLine(World, StartPosition, HitResult.Location, N_COLLISION_QUERY_HIT_COLOR, 
			false, Settings.Options.UpdateTimer, SDPG_World, Settings.Drawing.DrawLineThickness);
		DrawDebugPoint(World, HitResult.ImpactPoint, Settings.Drawing.DrawPointSize, N_COLLISION_QUERY_HIT_COLOR, 
			false, Settings.Options.UpdateTimer, SDPG_World);
	}
	else
	{
		DrawDebugLine(World, StartPosition, EndPosition, N_COLLISION_QUERY_MISS_COLOR, 
			false, Settings.Options.UpdateTimer, SDPG_World, Settings.Drawing.DrawLineThickness);
	}
#endif
}

void FNCollisionQueryTestUtils::DoLineTraceMulti(const FNCollisionQueryTestSettings& Settings, 
	const UWorld* World, const FVector& StartPosition, const FVector& EndPosition)
{
	N_COLLISION_QUERY_TEST_COMMON
	TArray<FHitResult> HitResults;
	
	if (Settings.Query.QueryBy == ECollisionQueryTestBy::Channel)
	{
		bHit = World->LineTraceMultiByChannel(HitResults, StartPosition, EndPosition, 
			Settings.Query.Channel, Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::ObjectType)
	{
		bHit = World->LineTraceMultiByObjectType(HitResults, StartPosition, EndPosition, 
			N_COLLISION_QUERY_OBJECT_PARAMS, Settings.Options.GetCollisionQueryParams());
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::Profile)
	{
		bHit = World->LineTraceMultiByProfile(HitResults, StartPosition, EndPosition, 
			Settings.Query.CollisionProfileName, Settings.Options.GetCollisionQueryParams());
	}
		
#if UE_ENABLE_DEBUG_DRAWING
	if (bHit)
	{
		// Adjust the end point to being the last hit point, unless it's by ObjectType.
		FVector TraceEndPosition = HitResults.Last().Location;
		if (Settings.Query.QueryBy == ECollisionQueryTestBy::ObjectType)
		{
			TraceEndPosition = EndPosition;
		}
		DrawDebugLine(World, StartPosition, TraceEndPosition, N_COLLISION_QUERY_HIT_COLOR, 
			false, Settings.Options.UpdateTimer, SDPG_World, Settings.Drawing.DrawLineThickness);
	}
	else
	{
		DrawDebugLine(World, StartPosition, EndPosition, 
			HitResults.Num() > 0 ? N_COLLISION_QUERY_MID_COLOR : N_COLLISION_QUERY_MISS_COLOR, 
			false, Settings.Options.UpdateTimer, SDPG_World, Settings.Drawing.DrawLineThickness);
	}
	for (const FHitResult& HitResult : HitResults)
	{
		DrawDebugPoint(World, HitResult.ImpactPoint, Settings.Drawing.DrawPointSize, 
			HitResult.bBlockingHit ? N_COLLISION_QUERY_HIT_COLOR : N_COLLISION_QUERY_MID_COLOR, 
			false, Settings.Options.UpdateTimer, SDPG_World);
	}
#endif
}

void FNCollisionQueryTestUtils::DoLineTraceTest(const FNCollisionQueryTestSettings& Settings, 
                                                const UWorld* World, const FVector& StartPosition, const FVector& EndPosition)
{
	N_COLLISION_QUERY_TEST_COMMON
	
	if (Settings.Query.QueryBy == ECollisionQueryTestBy::Channel)
	{
		bHit = World->LineTraceTestByChannel(StartPosition, EndPosition, 
			Settings.Query.Channel, Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::ObjectType)
	{
		bHit = World->LineTraceTestByObjectType(StartPosition, EndPosition, N_COLLISION_QUERY_OBJECT_PARAMS, 
			Settings.Options.GetCollisionQueryParams());
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::Profile)
	{
		bHit = World->LineTraceTestByProfile(StartPosition, EndPosition, Settings.Query.CollisionProfileName, Settings.Options.GetCollisionQueryParams());
	}

#if UE_ENABLE_DEBUG_DRAWING
	DrawDebugLine(World, StartPosition, EndPosition, 
		bHit ? N_COLLISION_QUERY_HIT_COLOR : N_COLLISION_QUERY_MISS_COLOR, 
		false, Settings.Options.UpdateTimer, SDPG_World, Settings.Drawing.DrawLineThickness);
#endif
}



void FNCollisionQueryTestUtils::DoSweepSingle(const FNCollisionQueryTestSettings& Settings, const UWorld* World, 
	const FVector& StartPosition, const FVector& EndPosition, const FQuat& Rotation)
{
	N_COLLISION_QUERY_TEST_COMMON
	const FCollisionShape CollisionShape = Settings.Query.GetCollisionShape();
	FHitResult HitResult;
	
	if (Settings.Query.QueryBy == ECollisionQueryTestBy::Channel)
	{
		bHit = World->SweepSingleByChannel(HitResult, StartPosition, EndPosition, Rotation, Settings.Query.Channel, 
			CollisionShape, Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::ObjectType)
	{
		bHit = World->SweepSingleByObjectType(HitResult, StartPosition, EndPosition, Rotation, 
			N_COLLISION_QUERY_OBJECT_PARAMS, CollisionShape, Settings.Options.GetCollisionQueryParams());
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::Profile)
	{
		bHit = World->SweepSingleByProfile(HitResult, StartPosition, EndPosition, Rotation, Settings.Query.CollisionProfileName, 
			CollisionShape, Settings.Options.GetCollisionQueryParams());
	}

#if UE_ENABLE_DEBUG_DRAWING
	if (bHit)
	{
		DrawDebugPoint(World, HitResult.ImpactPoint, Settings.Drawing.DrawPointSize, N_COLLISION_QUERY_HIT_COLOR, 
			false, Settings.Options.UpdateTimer, SDPG_World);
		FNDrawDebugHelpers::DrawSweep(World, StartPosition, HitResult.Location, Rotation, CollisionShape,
			N_COLLISION_QUERY_HIT_COLOR, false, Settings.Options.UpdateTimer, SDPG_World, 
			Settings.Drawing.DrawLineThickness);
	}
	else
	{
		FNDrawDebugHelpers::DrawSweep(World, StartPosition, EndPosition, Rotation, CollisionShape, 
			N_COLLISION_QUERY_MISS_COLOR, false, Settings.Options.UpdateTimer, SDPG_World, 
			Settings.Drawing.DrawLineThickness);
	}
#endif
}

void FNCollisionQueryTestUtils::DoSweepMulti(const FNCollisionQueryTestSettings& Settings, const UWorld* World, 
	const FVector& StartPosition, const FVector& EndPosition, const FQuat& Rotation)
{
	N_COLLISION_QUERY_TEST_COMMON
	const FCollisionShape CollisionShape = Settings.Query.GetCollisionShape();
	TArray<FHitResult> HitResults;
	
	if (Settings.Query.QueryBy == ECollisionQueryTestBy::Channel)
	{
		bHit = World->SweepMultiByChannel(HitResults, StartPosition, EndPosition, Rotation, Settings.Query.Channel, 
			CollisionShape, Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::ObjectType)
	{
		bHit = World->SweepMultiByObjectType(HitResults, StartPosition, EndPosition, Rotation, 
			N_COLLISION_QUERY_OBJECT_PARAMS, CollisionShape, Settings.Options.GetCollisionQueryParams());
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::Profile)
	{
		bHit = World->SweepMultiByProfile(HitResults, StartPosition, EndPosition, Rotation, Settings.Query.CollisionProfileName, 
			CollisionShape, Settings.Options.GetCollisionQueryParams());
	}

#if UE_ENABLE_DEBUG_DRAWING
	if (bHit)
	{
		FVector SweepEnd = HitResults.Last().Location;
		if (Settings.Query.QueryBy == ECollisionQueryTestBy::ObjectType)
		{
			SweepEnd = EndPosition;
		}
		else if (HitResults.Last().bStartPenetrating)
		{
			SweepEnd = EndPosition;
		}

		FNDrawDebugHelpers::DrawSweep(World, StartPosition, SweepEnd, Rotation, CollisionShape, 
			N_COLLISION_QUERY_HIT_COLOR, false, Settings.Options.UpdateTimer, SDPG_World, 
			Settings.Drawing.DrawLineThickness);
	}
	else
	{
		FNDrawDebugHelpers::DrawSweep(World, StartPosition, EndPosition, Rotation, CollisionShape, 
			HitResults.Num() > 0 ? N_COLLISION_QUERY_MID_COLOR : N_COLLISION_QUERY_MISS_COLOR, 
			false, Settings.Options.UpdateTimer, SDPG_World, Settings.Drawing.DrawLineThickness);
	}

	for (const FHitResult& HitResult : HitResults)
	{
		DrawDebugPoint(World, HitResult.ImpactPoint, Settings.Drawing.DrawPointSize, 
			HitResult.bBlockingHit ? N_COLLISION_QUERY_HIT_COLOR : N_COLLISION_QUERY_MID_COLOR, 
			false, Settings.Options.UpdateTimer, SDPG_World);
	}
#endif
}

void FNCollisionQueryTestUtils::DoSweepTest(const FNCollisionQueryTestSettings& Settings, const UWorld* World, const FVector& StartPosition, const FVector& EndPosition, const FQuat& Rotation)
{
	N_COLLISION_QUERY_TEST_COMMON
	const FCollisionShape& CollisionShape = Settings.Query.GetCollisionShape();
	
	if (Settings.Query.QueryBy == ECollisionQueryTestBy::Channel)
	{
		bHit = World->SweepTestByChannel(StartPosition, EndPosition, Rotation, Settings.Query.Channel, CollisionShape, 
			Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::ObjectType)
	{
		bHit = World->SweepTestByObjectType(StartPosition, EndPosition, Rotation, N_COLLISION_QUERY_OBJECT_PARAMS, 
			CollisionShape, Settings.Options.GetCollisionQueryParams());
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::Profile)
	{
		bHit = World->SweepTestByProfile(StartPosition, EndPosition, Rotation, Settings.Query.CollisionProfileName, 
			CollisionShape, Settings.Options.GetCollisionQueryParams());
	}

#if UE_ENABLE_DEBUG_DRAWING
	FNDrawDebugHelpers::DrawSweep(World, StartPosition, EndPosition, Rotation, CollisionShape, 
		bHit ? N_COLLISION_QUERY_HIT_COLOR: N_COLLISION_QUERY_MISS_COLOR, false, 
		Settings.Options.UpdateTimer, SDPG_World, Settings.Drawing.DrawLineThickness);
#endif
}

void FNCollisionQueryTestUtils::DoOverlapBlocking(const FNCollisionQueryTestSettings& Settings, const UWorld* World,
	const FVector& Position, const FQuat& Rotation)
{
	N_COLLISION_QUERY_TEST_COMMON
	const FCollisionShape& CollisionShape = Settings.Query.GetCollisionShape();
	
	if (Settings.Query.QueryBy == ECollisionQueryTestBy::Channel)
	{
		bHit = World->OverlapBlockingTestByChannel(Position, Rotation, Settings.Query.Channel, CollisionShape, 
			Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::ObjectType)
	{
		bHit = World->OverlapAnyTestByObjectType(Position, Rotation, N_COLLISION_QUERY_OBJECT_PARAMS, 
			CollisionShape, Settings.Options.GetCollisionQueryParams());
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::Profile)
	{
		bHit = World->OverlapBlockingTestByProfile(Position, Rotation, Settings.Query.CollisionProfileName, CollisionShape, 
			Settings.Options.GetCollisionQueryParams());
	}

#if UE_ENABLE_DEBUG_DRAWING
	FNDrawDebugHelpers::DrawCollisionShape(World, Position, Rotation, CollisionShape, 
		bHit ? N_COLLISION_QUERY_HIT_COLOR : N_COLLISION_QUERY_MISS_COLOR, false, Settings.Options.UpdateTimer, 
		SDPG_World, Settings.Drawing.DrawLineThickness);
#endif 
}

void FNCollisionQueryTestUtils::DoOverlapAny(const FNCollisionQueryTestSettings& Settings, 
	const UWorld* World, const FVector& Position, const FQuat& Rotation)
{
	N_COLLISION_QUERY_TEST_COMMON
	const FCollisionShape& CollisionShape = Settings.Query.GetCollisionShape();
	
	if (Settings.Query.QueryBy == ECollisionQueryTestBy::Channel)
	{
		bHit = World->OverlapAnyTestByChannel(Position, Rotation, Settings.Query.Channel, CollisionShape, 
			Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::ObjectType)
	{
		bHit = World->OverlapAnyTestByObjectType(Position, Rotation, N_COLLISION_QUERY_OBJECT_PARAMS, 
			CollisionShape, Settings.Options.GetCollisionQueryParams());
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::Profile)
	{
		bHit = World->OverlapAnyTestByProfile(Position, Rotation, Settings.Query.CollisionProfileName, CollisionShape, 
			Settings.Options.GetCollisionQueryParams());
	}

#if UE_ENABLE_DEBUG_DRAWING
	FNDrawDebugHelpers::DrawCollisionShape(World, Position, Rotation, CollisionShape, 
		bHit ? N_COLLISION_QUERY_HIT_COLOR : N_COLLISION_QUERY_MISS_COLOR, false, Settings.Options.UpdateTimer, 
		SDPG_World, Settings.Drawing.DrawLineThickness);
#endif
}

void FNCollisionQueryTestUtils::DoOverlapMulti(const FNCollisionQueryTestSettings& Settings, 
	const UWorld* World, const FVector& Position, const FQuat& Rotation)
{
	N_COLLISION_QUERY_TEST_COMMON
	TArray<FOverlapResult>OverlapsResults;
	const FCollisionShape& CollisionShape = Settings.Query.GetCollisionShape();
	
	if (Settings.Query.QueryBy == ECollisionQueryTestBy::Channel)
	{
		bHit = World->OverlapMultiByChannel(OverlapsResults, Position, Rotation, Settings.Query.Channel, CollisionShape, 
			Settings.Options.GetCollisionQueryParams(), Settings.Query.GetCollisionResponseParams());
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::ObjectType)
	{
		bHit = World->OverlapMultiByObjectType(OverlapsResults, Position, Rotation, 
			N_COLLISION_QUERY_OBJECT_PARAMS, CollisionShape, Settings.Options.GetCollisionQueryParams());
	}
	else if (Settings.Query.QueryBy == ECollisionQueryTestBy::Profile)
	{
		bHit = World->OverlapMultiByProfile(OverlapsResults, Position, Rotation, Settings.Query.CollisionProfileName, 
			CollisionShape, Settings.Options.GetCollisionQueryParams());
	}

#if UE_ENABLE_DEBUG_DRAWING
	if (Settings.Drawing.DrawLabels)
	{
		for (const FOverlapResult& OverlapResult : OverlapsResults)
		{
		
		}
	}
	FNDrawDebugHelpers::DrawCollisionShape(World, Position, Rotation, CollisionShape, 
		bHit ? N_COLLISION_QUERY_HIT_COLOR : N_COLLISION_QUERY_MISS_COLOR, false, Settings.Options.UpdateTimer, 
		SDPG_World, Settings.Drawing.DrawLineThickness);
#endif
}