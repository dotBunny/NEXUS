// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "INProcGenOperationOwner.h"
#include "NEditorUtils.h"
#include "NProcGenOperation.h"
#include "Macros/NEditorSubsystemMacros.h"
#include "NProcGenEditorSubsystem.generated.h"

class ANCellProxy;
class UNProcGenOperation;

UCLASS()
class NEXUSPROCGENEDITOR_API UNProcGenEditorSubsystem : public UEditorSubsystem, public FTickableGameObject, public INProcGenOperationOwner
{
	GENERATED_BODY()
	N_EDITOR_TICKABLE_SUBSYSTEM(UNProcGenEditorSubsystem)



	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override
	{
		for (const auto Operation : KnownOperations)
		{
			Operation->Tick();
		}
		
		LastFrameNumberWeTicked = GFrameCounter;
	}
	
	virtual bool IsTickable() const override
	{
		if (!HasKnownOperation() || 
			LastFrameNumberWeTicked == GFrameCounter ||
			FNEditorUtils::IsEditorShuttingDown())
		{
			return false;
		}
		
		return true;
	}
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UNProcGenEditorSubsystem, STATGROUP_Tickables);
	}
	virtual bool IsTickableWhenPaused() const override{ return true; }
	virtual bool IsTickableInEditor() const override { return true; }

	virtual void StartOperation(UNProcGenOperation* Operation) override;
	virtual void OnOperationFinished(UNProcGenOperation* Operation, TSharedRef<FNProcGenOperationSharedContext> SharedContext) override;
	virtual void OnOperationDestroyed(UNProcGenOperation* Operation) override;
	virtual UWorld* GetDefaultWorld() override { return FNEditorUtils::GetCurrentWorld(); };
	
	bool HasKnownOperation() const { return KnownOperations.Num() > 0; }
	bool HasGeneratedCellProxies() const { return ProxyMap.Num() > 0; }
	
	void ClearAllProxies();
	
	void ClearGenerated(const FName& Key)
	{
		ClearGeneratedProxies(Key);
	}
	
	void ClearGeneratedProxies(const FName& Key);

	
	void LoadAllGeneratedProxies();
	void LoadGeneratedProxies(const FName& Key);
	void UnloadAllGeneratedProxies();
	void UnloadGeneratedProxies(const FName& Key);
	
protected:
	void OnPreBeginPIE(bool bArg);
	void OnMapLoad(const FString& String, FCanLoadMap& CanLoadMap);
private:

	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	
	UPROPERTY()
	TArray<TObjectPtr<UNProcGenOperation>> KnownOperations;

	/**
	 * An array of all known proxies that have been put into the level, this ensures that they are not
	 * garbage collected and that their spawned level instances are protected as well. We have to manage
	 * them directly for cases like going into PIE and loading a different level.
	 */
	UPROPERTY()
	TArray<TObjectPtr<ANCellProxy>> KnownProxies;
	
	TMap<FName, TArray<ANCellProxy*>> ProxyMap;
	
	uint32 LastFrameNumberWeTicked = INDEX_NONE;
	
	FDelegateHandle OnMapLoadHandle;
	FDelegateHandle PreBeginPIEHandle;
};


// TODO: Clear generated stuff going into play?