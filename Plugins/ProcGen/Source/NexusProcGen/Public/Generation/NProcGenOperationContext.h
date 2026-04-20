// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NProcGenOperationSettings.h"
#include "NProcGenOperationBoneContext.h"
#include "NProcGenOperationOrganContext.h"
#include "Organ/NBoneComponent.h"

class UNOrganComponent;


/**
 * Game-thread context of the entire generation process.
 */
class NEXUSPROCGEN_API FNProcGenOperationContext
{
	friend class UNProcGenOperation;
public:
	explicit FNProcGenOperationContext(uint32 NewOperationTicket);
	
	TArray<UNOrganComponent*> InputComponents;
	TMap<UNOrganComponent*, FNProcGenOperationOrganContext> OrganContext;
	TMap<UNBoneComponent*, FNProcGenOperationBoneContext> BoneContext;
	
	TMap<UNOrganComponent*, TArray<UNBoneComponent*>> ComponentBoneMap;
	TArray<TArray<UNOrganComponent*>> GenerationOrder;
	
	bool AddOrganComponent(UNOrganComponent* Component);
	void SetOperationSettings(const FNProcGenOperationSettings& InSettings);
	bool IsLocked() const { return bIsLocked; }
	void LockAndPreprocess(UWorld* World);
	void OutputToLog(bool bBuildTissues = false);
	void ResetContext();
	
	void SetTargetWorld(UWorld* World) { TargetWorld = World; }
	UWorld* GetTargetWorld() const { return TargetWorld; }
	uint32 GetOperationTicket() const { return OperationTicket; }
	const FNProcGenOperationSettings& GetOperationSettings() { return OperationSettings; }
	
private:
	/**
	 * The identifier of the UNProcGenOperation that owns this context.
	 */
	uint32 OperationTicket;

	FNProcGenOperationSettings OperationSettings;
	bool bIsLocked = false;
	UWorld* TargetWorld = nullptr;
};
