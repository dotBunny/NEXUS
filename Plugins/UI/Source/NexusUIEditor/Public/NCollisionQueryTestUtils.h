// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NCollisionQueryTestSettings.h"


class FNCollisionQueryTestUtils
{
public:
	
	static void DoLineTraceSingle(const FNCollisionQueryTestSettings& Settings, 
		const UWorld* World, const FVector& StartPosition, const FVector& EndPosition);
	
	static void DoLineTraceMulti(const FNCollisionQueryTestSettings& Settings, 
	const UWorld* World, const FVector& StartPosition, const FVector& EndPosition);
	
	static void DoLineTraceTest(const FNCollisionQueryTestSettings& Settings,
		const UWorld* World, const FVector& StartPosition, const FVector& EndPosition);
	
	static void DoSweepSingle(const FNCollisionQueryTestSettings& Settings, 
		const UWorld* World, const FVector& StartPosition, const FVector& EndPosition, const FQuat& Rotation);
	
	static void DoSweepMulti(const FNCollisionQueryTestSettings& Settings, 
		const UWorld* World, const FVector& StartPosition, const FVector& EndPosition, const FQuat& Rotation);
	
	static void DoSweepTest(const FNCollisionQueryTestSettings& Settings, 
		const UWorld* World, const FVector& StartPosition, const FVector& EndPosition, const FQuat& Rotation);
	
	static void DoOverlapBlocking(const FNCollisionQueryTestSettings& Settings, 
		const UWorld* World, const FVector& Position, const FQuat& Rotation);
	
	static void DoOverlapAny(const FNCollisionQueryTestSettings& Settings, 
		const UWorld* World, const FVector& Position, const FQuat& Rotation);
	
	static void DoOverlapMulti(const FNCollisionQueryTestSettings& Settings, 
		const UWorld* World, const FVector& Position, const FQuat& Rotation);
	
	static TArray<FName> GetCollisionProfileNames()
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
	
private:
	static EQueryMobilityType ToQueryMobilityType(const ECollisionQueryTestMobility Mobility)
	{
		switch (Mobility)
		{
			using enum ECollisionQueryTestMobility;
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
