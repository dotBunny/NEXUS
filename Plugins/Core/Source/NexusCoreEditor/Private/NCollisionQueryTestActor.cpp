// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCollisionQueryTestActor.h"
#include "NColor.h"

#define N_COLLISION_QUERY_TEST_COMMON \
	const FVector StartPosition = GetActorLocation(); \
	const FVector EndPosition = EndPointComponent->GetComponentLocation(); \
	bool bHit = false; \
	const UWorld* World = GetWorld();
#define N_COLLISION_QUERY_HIT_COLOR FNColor::GetColor(ENColor::NC_Green)
#define N_COLLISION_QUERY_MID_COLOR FNColor::GetColor(ENColor::NC_BlueMid)
#define N_COLLISION_QUERY_MISS_COLOR FNColor::GetColor(ENColor::NC_Red)


ANCollisionQueryTestActor::ANCollisionQueryTestActor(const FObjectInitializer& ObjectInitializer)
{
	bIsEditorOnlyActor = true;
	
	StartPointComponent= CreateDefaultSubobject<USceneComponent>(TEXT("Start Point"));
	StartPointComponent->SetMobility(EComponentMobility::Movable);
	StartPointComponent->bIsEditorOnly = true;
#if WITH_EDITORONLY_DATA
	StartPointComponent->bVisualizeComponent = true;
#endif	
	
	RootComponent = StartPointComponent;
	
	EndPointComponent= CreateDefaultSubobject<USceneComponent>(TEXT("End Point"));
	EndPointComponent->SetMobility(EComponentMobility::Movable);
	EndPointComponent->SetupAttachment(RootComponent);
	EndPointComponent->SetRelativeLocation(FVector(500.f, 0.f, 0.f));
	EndPointComponent->bIsEditorOnly = true;
#if WITH_EDITORONLY_DATA
	EndPointComponent->bVisualizeComponent = true;
#endif
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ANCollisionQueryTestActor::Tick(const float DeltaSeconds)
{
	TickTimer -= DeltaSeconds;
	if (TickTimer <= 0)
	{
		Query(UpdateTimer);
		TickTimer = UpdateTimer;
	}
	
	Super::Tick(DeltaSeconds);	
}

TArray<FName> ANCollisionQueryTestActor::GetCollisionProfileNames()
{
	TArray<FName> Names;
	const UCollisionProfile* CollisionProfile = UCollisionProfile::Get();
	const int32 Count = CollisionProfile->GetNumOfProfiles();
	Names.Reserve(Count);

	for (int32 i = 0; i < Count; ++i)
	{
		const FCollisionResponseTemplate* ProfileTemplate = CollisionProfile->GetProfileByIndex(i);
		Names.Add(ProfileTemplate->Name);
	}

	return MoveTemp(Names);
}

void ANCollisionQueryTestActor::Query(const float DrawTime) const
{
	if (QueryMethod == ECollisionQueryTestMethod::LineTrace)
	{
		if (QueryPrefix == ECollisionQueryTestPrefix::Single)
		{
			DoLineTraceSingle(DrawTime);
		}
		else if (QueryPrefix == ECollisionQueryTestPrefix::Multi)
		{
			DoLineTraceMulti(DrawTime);
		}
		else if (QueryPrefix == ECollisionQueryTestPrefix::Test)
		{
			DoLineTraceTest(DrawTime);
		}
	}
	else if (QueryMethod == ECollisionQueryTestMethod::Sweep)
	{
	
	}
	else if (QueryMethod == ECollisionQueryTestMethod::Overlap)
	{
	
	}
}

void ANCollisionQueryTestActor::DoLineTraceSingle(const float DrawTime) const
{
	N_COLLISION_QUERY_TEST_COMMON
	FHitResult HitResult;
	
	if (QueryBy == ECollisionQueryTestBy::Channel)
	{
		bHit = World->LineTraceSingleByChannel(HitResult, StartPosition, EndPosition, 
			Channel, GetCollisionQueryParams(), GetCollisionResponseParams());
	}
	else if (QueryBy == ECollisionQueryTestBy::ObjectType)
	{
		bHit = World->LineTraceSingleByObjectType(HitResult, StartPosition, EndPosition, 
			FCollisionObjectQueryParams(UEngineTypes::ConvertToCollisionChannel(ObjectType)));
	}
	else if (QueryBy == ECollisionQueryTestBy::Profile)
	{
		bHit = World->LineTraceSingleByProfile(HitResult, StartPosition, EndPosition, 
			CollisionProfileName, GetCollisionQueryParams());
	}
		
#if UE_ENABLE_DEBUG_DRAWING
	if (bHit)
	{
		DrawDebugLine(World, StartPosition, HitResult.Location, N_COLLISION_QUERY_HIT_COLOR, 
			false, DrawTime, SDPG_World, DrawLineThickness);
		DrawDebugPoint(World, HitResult.ImpactPoint, DrawPointSize, N_COLLISION_QUERY_HIT_COLOR, 
			false, DrawTime, SDPG_World);
	}
	else
	{
		DrawDebugLine(World, StartPosition, EndPosition, N_COLLISION_QUERY_MISS_COLOR, 
			false, DrawTime, SDPG_World, DrawLineThickness);
	}
#endif
}

void ANCollisionQueryTestActor::DoLineTraceMulti(const float DrawTime) const
{
	N_COLLISION_QUERY_TEST_COMMON
	TArray<FHitResult> HitResults;
	if (QueryBy == ECollisionQueryTestBy::Channel)
	{
		bHit = World->LineTraceMultiByChannel(HitResults, StartPosition, EndPosition, 
			Channel, GetCollisionQueryParams(), GetCollisionResponseParams());
	}
	else if (QueryBy == ECollisionQueryTestBy::ObjectType)
	{
		bHit = World->LineTraceMultiByObjectType(HitResults, StartPosition, EndPosition, 
			FCollisionObjectQueryParams(UEngineTypes::ConvertToCollisionChannel(ObjectType)), GetCollisionQueryParams());
	}
	else if (QueryBy == ECollisionQueryTestBy::Profile)
	{
		bHit = World->LineTraceMultiByProfile(HitResults, StartPosition, EndPosition, 
			CollisionProfileName, GetCollisionQueryParams());
	}
		
#if UE_ENABLE_DEBUG_DRAWING
	if (bHit)
	{
		// Adjust the end point to being the last hit point, unless it's by ObjectType.
		FVector TraceEndPosition = HitResults.Last().Location;
		if (QueryBy == ECollisionQueryTestBy::ObjectType)
		{
			TraceEndPosition = EndPosition;
		}
		DrawDebugLine(World, StartPosition, TraceEndPosition, N_COLLISION_QUERY_HIT_COLOR, 
			false, DrawTime, SDPG_World, DrawLineThickness);
	}
	else
	{
		DrawDebugLine(World, StartPosition, EndPosition, 
			HitResults.Num() > 0 ? N_COLLISION_QUERY_MID_COLOR : N_COLLISION_QUERY_MISS_COLOR, 
			false, DrawTime, SDPG_World, DrawLineThickness);
	}
	for (const FHitResult& HitResult : HitResults)
	{
		DrawDebugPoint(World, HitResult.ImpactPoint, DrawPointSize, 
			HitResult.bBlockingHit ? N_COLLISION_QUERY_HIT_COLOR : N_COLLISION_QUERY_MID_COLOR, 
			false, DrawTime, SDPG_World);
	}
#endif
}

void ANCollisionQueryTestActor::DoLineTraceTest(const float DrawTime) const
{
	N_COLLISION_QUERY_TEST_COMMON
	if (QueryBy == ECollisionQueryTestBy::Channel)
	{
		bHit = World->LineTraceTestByChannel(StartPosition, EndPosition, 
			Channel, GetCollisionQueryParams(), GetCollisionResponseParams());
	}
	else if (QueryBy == ECollisionQueryTestBy::ObjectType)
	{
		bHit = World->LineTraceTestByObjectType(StartPosition, EndPosition, 
			FCollisionObjectQueryParams(UEngineTypes::ConvertToCollisionChannel(ObjectType)), GetCollisionQueryParams());
	}
	else if (QueryBy == ECollisionQueryTestBy::Profile)
	{
		bHit = World->LineTraceTestByProfile(StartPosition, EndPosition, CollisionProfileName, GetCollisionQueryParams());
	}
#if UE_ENABLE_DEBUG_DRAWING
	DrawDebugLine(World, StartPosition, EndPosition, 
		bHit ? N_COLLISION_QUERY_HIT_COLOR : N_COLLISION_QUERY_MISS_COLOR, 
		false, DrawTime, SDPG_World, DrawLineThickness);
#endif
}

void ANCollisionQueryTestActor::DoSweepSingle(const float DrawTime) const
{
}

void ANCollisionQueryTestActor::DoSweepMulti(const float DrawTime) const
{
}

void ANCollisionQueryTestActor::DoSweepTest(const float DrawTime) const
{
}

void ANCollisionQueryTestActor::DoOverlapSingle(const float DrawTime) const
{
}

void ANCollisionQueryTestActor::DoOverlapMulti(const float DrawTime) const
{
}

void ANCollisionQueryTestActor::DoOverlapTest(const float DrawTime) const
{
}

FCollisionQueryParams ANCollisionQueryTestActor::GetCollisionQueryParams() const
{
	FCollisionQueryParams Parameters;
	Parameters.bTraceComplex = bTraceComplex;
	Parameters.bFindInitialOverlaps = bFindInitialOverlaps;
	Parameters.bIgnoreBlocks = bIgnoreBlocks;
	Parameters.bIgnoreTouches = bIgnoreTouches;
	Parameters.bSkipNarrowPhase = bSkipNarrowPhase;
	return MoveTemp(Parameters);
}

FCollisionResponseParams ANCollisionQueryTestActor::GetCollisionResponseParams() const
{
	FCollisionResponseParams Parameters;
	Parameters.CollisionResponse = CollisionResponses;
	return MoveTemp(Parameters);
}

FCollisionShape ANCollisionQueryTestActor::GetCollisionShape() const
{
	switch (QueryShape)
	{
		using enum ECollisionQueryTestShape;
	case Box:
		return FCollisionShape::MakeBox(ShapeBoxHalfExtents);
	case Capsule:
		return FCollisionShape::MakeCapsule(ShapeCapsuleRadius, ShapeCapsuleHalfHeight);
	case Sphere:
		return FCollisionShape::MakeSphere(SphereRadius);
	}
	return FCollisionShape();
}
