// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NActorUtils.h"
#include "NCreateWorldContext.h"
#include "Organ/NOrganVolume.h"

class FNCreateWorldTask
{
public:
	explicit FNCreateWorldTask(const TSharedPtr<FNCreateWorldContext>& CreateWorldContextPtr) 
	: CreateWorldContextPtr(CreateWorldContextPtr.ToSharedRef())
	{
		
	}

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FNCreateWorldTask, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
	
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);
	
	static FNWorldActorFilterSettings CreateWorldActorFilterSettings()
	{
		// Collect the world AActors that we need to care about
		FNWorldActorFilterSettings ActorFilterSettings;
		ActorFilterSettings.bExcludeNonCollisionEnabledActors = true;
		ActorFilterSettings.bIncludePlayerStarts = true;
		ActorFilterSettings.ExclusionFunction = &IsWorldCollisionSource;
		return MoveTemp(ActorFilterSettings);
	}
	
	/** Actor filter for world-collision gathering. */
	static bool IsWorldCollisionSource(const AActor* Actor)
	{
		if (Actor->IsA<ANOrganVolume>()) return false;
		return true;
	}
private:
	TSharedRef<FNCreateWorldContext> CreateWorldContextPtr;;
};