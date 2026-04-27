// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Generation/Contexts/NWorldContext.h"

class FNProcessWorldContextTask
{
public:
	explicit FNProcessWorldContextTask(const TSharedPtr<FNWorldContext>& WorldContextPtr) 
	: WorldContextPtr(WorldContextPtr.ToSharedRef())
	{
		
	}

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNProcessWorldContextTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::AnyThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
	
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
	{
		int32 Count = WorldContextPtr->CollisionMeshTransforms.Num();
		
		WorldContextPtr->CollisionMeshLocations.Empty();
		WorldContextPtr->CollisionMeshLocations.Reserve(Count);
		
		WorldContextPtr->CollisionMeshRotations.Empty();
		WorldContextPtr->CollisionMeshRotations.Reserve(Count);
		
		// Cache our used data for later
		for (FTransform Transform : WorldContextPtr->CollisionMeshTransforms)
		{
			WorldContextPtr->CollisionMeshLocations.Add(Transform.GetLocation());
			WorldContextPtr->CollisionMeshRotations.Add(Transform.GetRotation().Rotator());
		}
	}
	
private:
	TSharedRef<FNWorldContext> WorldContextPtr;;
};