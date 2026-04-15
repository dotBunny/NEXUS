// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NProcGenOperation.h"
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
	
	TArray<UNOrganComponent*> InputComponents;
	TMap<UNOrganComponent*, FNProcGenOperationOrganContext> OrganContext;
	TMap<UNBoneComponent*, FNProcGenOperationBoneContext> BoneContext;
	
	TMap<UNOrganComponent*, TArray<UNBoneComponent*>> ComponentBoneMap;
	TArray<TArray<UNOrganComponent*>> GenerationOrder;
	
	FString FriendlySeed;
	
	bool AddOrganComponent(UNOrganComponent* Component);
	void ApplySettings(const FNProcGenOperationSettings& Settings);
	bool IsLocked() const { return bIsLocked; }
	void LockAndPreprocess(UWorld* World);
	void OutputToLog(bool bBuildTissues = false);
	void ResetContext();
	
	void SetTargetWorld(UWorld* World) { TargetWorld = World; }
	UWorld* GetTargetWorld() const { return TargetWorld; }
	
	void SetDisplayName(FString InDisplayName) { DisplayName = InDisplayName; }
	
	bool GetCreateLevelInstances() const { return bCreateLevelInstances; }
	bool GetLoadLevelInstances() const { return bLoadLevelInstances; }
	bool GetReplicateLevelInstances() const { return bReplicateLevelInstances; }
	
private:
	
	bool bCreateLevelInstances = false;
	bool bLoadLevelInstances = false;
	bool bReplicateLevelInstances = true;
	
	bool bIsLocked = false;
	UWorld* TargetWorld = nullptr;
	FString DisplayName;
};
