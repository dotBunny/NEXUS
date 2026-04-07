// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NProcGenOperationBoneContext.h"
#include "NProcGenOperationOrganContext.h"
#include "Organ/NBoneComponent.h"

class UNOrganComponent;


/**
 * Game-thread context of the entire generation process.
 */
class NEXUSPROCGEN_API FNProcGenOperationContext
{
public:
	
	TMap<UNOrganComponent*, FNProcGenOperationOrganContext> OrganContext;
	TMap<UNBoneComponent*, FNProcGenOperationBoneContext> BoneContext;
	
	TMap<UNOrganComponent*, TArray<UNBoneComponent*>> ComponentBoneMap;
	TArray<TArray<UNOrganComponent*>> GenerationOrder;
	
	FString FriendlySeed;
	
	bool AddOrganComponent(UNOrganComponent* Component);
	bool IsLocked() const { return bIsLocked; }
	void LockAndPreprocess(UWorld* World);
	void OutputToLog();
	void ResetContext();
	void SetTargetWorld(UWorld* World) { TargetWorld = World; }
	UWorld* GetTargetWorld() const { return TargetWorld; }
	void SetDisplayName(FString InDisplayName) { DisplayName = InDisplayName; }
	
private:
	
	bool bIsLocked = false;
	UWorld* TargetWorld = nullptr;
	FString DisplayName;
};
