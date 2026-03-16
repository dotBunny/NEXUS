// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorUtils.h"
#include "Macros/NEditorSubsystemMacros.h"
#include "NMultiplayerEditorSubsystem.generated.h"

UCLASS()
class NEXUSMULTIPLAYEREDITOR_API UNMultiplayerEditorSubsystem : public UEditorSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	N_EDITOR_TICKABLE_SUBSYSTEM(UNMultiplayerEditorSubsystem)

	virtual void Tick(float DeltaTime) override;
	
	virtual bool IsTickable() const override
	{
		if (FNEditorUtils::IsEditorShuttingDown())
		{
			return false;
		}
		return bIsMultiplayerTestRunning && ProcessHandles.Num() > 0;
	}
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UNProcGenEditorSubsystem, STATGROUP_Tickables);
	}
	virtual bool IsTickableWhenPaused() const override{ return true; }
	virtual bool IsTickableInEditor() const override { return true; }
	
	void StartMultiplayerTest();
	void StopMultiplayerTest();
	void ToggleMultiplayerTest();
	
	bool IsTestRunning() const {  return bIsMultiplayerTestRunning; }
	
	UFUNCTION()
	void AddLocalProcess(const uint32 ProcessIdentifier);
	
private:
	FDelegateHandle LocalProcessDelegateHandle;
	TArray<FProcHandle> ProcessHandles;
	bool bIsMultiplayerTestRunning;
	
	float DeltaAccumulator;

};