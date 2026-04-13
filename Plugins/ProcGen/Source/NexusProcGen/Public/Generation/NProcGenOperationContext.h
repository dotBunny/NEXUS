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
	
	TArray<UNOrganComponent*> InputComponents;
	TMap<UNOrganComponent*, FNProcGenOperationOrganContext> OrganContext;
	TMap<UNBoneComponent*, FNProcGenOperationBoneContext> BoneContext;
	
	TMap<UNOrganComponent*, TArray<UNBoneComponent*>> ComponentBoneMap;
	TArray<TArray<UNOrganComponent*>> GenerationOrder;
	
	FString FriendlySeed;
	
	bool AddOrganComponent(UNOrganComponent* Component);
	bool IsLocked() const { return bIsLocked; }
	void LockAndPreprocess(UWorld* World);
	void OutputToLog(bool bBuildTissues = false);
	void ResetContext();
	
	void SetTargetWorld(UWorld* World) { TargetWorld = World; }
	UWorld* GetTargetWorld() const { return TargetWorld; }
	
	void SetDisplayName(FString InDisplayName) { DisplayName = InDisplayName; }
	
	bool GetCreateInstances() const { return bCreateLevelInstances; }
	void SetCreateInstances(const bool bCreateInstances) { bCreateLevelInstances = bCreateInstances; }
	bool GetLoadInstances() const { return bLoadLevelInstances; }
	void SetLoadInstances(const bool bLoadInstances) { bLoadLevelInstances = bLoadInstances; }
	
private:
	
	bool bCreateLevelInstances = false;
	bool bLoadLevelInstances = false;
	
	bool bIsLocked = false;
	UWorld* TargetWorld = nullptr;
	FString DisplayName;
};
