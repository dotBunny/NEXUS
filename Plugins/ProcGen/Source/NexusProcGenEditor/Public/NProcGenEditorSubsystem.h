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

// TODO: Is there a way to save the keys and pass them through to playmode based on the last generation? so 
// a user could generate and then visually see in game what they were hammering to
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
	virtual void OnOperationFinished(UNProcGenOperation* Operation, TSharedRef<FNProcGenTaskGraphContext> TaskGraphContext) override;
	virtual void OnOperationDestroyed(UNProcGenOperation* Operation) override;
	virtual UWorld* GetDefaultWorld() override { return FNEditorUtils::GetCurrentWorld(); };
	
	bool HasKnownOperation() const { return KnownOperations.Num() > 0; }
	bool HasGeneratedCellProxies() const { return KnownProxies.Num() > 0; }
	
	void ClearAllProxies();
	void ClearGenerated(const FName& Key);
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
	 * garbage collected and that their spawned level instances are protected as well. However, because 
	 * they are marked transient we need to manage their cleanup explicitly when going into things like PIE
	 * or transitioning to new levels.
	 */
	UPROPERTY()
	TArray<TObjectPtr<ANCellProxy>> KnownProxies;
	
	TMap<FName, TArray<ANCellProxy*>> ProxyMap;
	
	uint32 LastFrameNumberWeTicked = INDEX_NONE;
	
	FDelegateHandle OnMapLoadHandle;
	FDelegateHandle PreBeginPIEHandle;
};