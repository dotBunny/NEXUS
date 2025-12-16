// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NProcGenOperation.h"
#include "Macros/NSubsystemMacros.h"
#include "NProcGenEditorSubsystem.generated.h"

class UNProcGenOperation;

UCLASS()
class NEXUSPROCGENEDITOR_API UNProcGenEditorSubsystem : public UEditorSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	N_EDITOR_SUBSYSTEM(UNProcGenEditorSubsystem)
	
	virtual void Tick(float DeltaTime) override
	{
		if (LastFrameNumberWeTicked == GFrameCounter || KnownOperations.Num() == 0)
			return;
		
		for (const auto Operation : KnownOperations)
		{
			Operation->Tick();
		}
		
		LastFrameNumberWeTicked = GFrameCounter;
	}
	
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UNProcGenEditorSubsystem, STATGROUP_Tickables);
	}
	virtual bool IsTickableWhenPaused() const override{ return true; }
	virtual bool IsTickableInEditor() const override { return true; }
	
	void StartOperation(UNProcGenOperation* Operation);
	void OnOperationFinished(UNProcGenOperation* Operation);
	bool HasKnownOperation() const { return KnownOperations.Num() > 0; }


private:
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TArray<UNProcGenOperation*> KnownOperations;
	
	uint32 LastFrameNumberWeTicked = INDEX_NONE;
};