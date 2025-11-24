// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NSubsystemMacros.h"
#include "NProcGenEditorSubsystem.generated.h"

class UNOrganGenerator;

UCLASS()
class NEXUSPROCGENEDITOR_API UNProcGenEditorSubsystem : public UEditorSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	N_EDITOR_SUBSYSTEM(UNProcGenEditorSubsystem)
	
	virtual void Tick(float DeltaTime) override
	{
		if (LastFrameNumberWeTicked == GFrameCounter || ActiveGenerators.Num() == 0)
			return;
		
		
		// TODO: Logic
		
		LastFrameNumberWeTicked = GFrameCounter;
	}
	
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UNProcGenEditorSubsystem, STATGROUP_Tickables);
	}
	virtual bool IsTickableWhenPaused() const override{ return true; }
	virtual bool IsTickableInEditor() const override { return true; }
	
	void BuildGenerator(UNOrganGenerator* OrganGenerator);
	auto OnFinishedBuild(UNOrganGenerator* OrganGenerator) -> void;

private:
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TArray<UNOrganGenerator*> ActiveGenerators;
	
	uint32 LastFrameNumberWeTicked = INDEX_NONE;
};