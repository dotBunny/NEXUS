// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Organ/NBoneComponent.h"


class UNOrganComponent;

struct NEXUSPROCGEN_API FNOrganGenerationContext
{
	UNOrganComponent* SourceComponent;
	TArray<UNOrganComponent*> IntersectComponents;
	TArray<UNOrganComponent*> ContainedComponents;
};

/**
 * Game-thread context of the entire generation process.
 */
class NEXUSPROCGEN_API FNProcGenOperationContext
{
public:
	
	TMap<UNOrganComponent*, FNOrganGenerationContext> Components;
	TArray<TArray<UNOrganComponent*>> GenerationOrder;
	FString FriendlySeed;
	
	bool AddOrganComponent(UNOrganComponent* Component);
	bool IsLocked() const { return bIsLocked; }
	void LockAndPreprocess(UWorld* World);
	void OutputToLog();
	void ResetContext();
	void SetTargetWorld(UWorld* World) { TargetWorld = World; }

private:

	bool bIsLocked = false;
	UWorld* TargetWorld = nullptr;
};
