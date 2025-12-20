// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class UNProcGenOperation;
class FNProcGenOperationContext;

class NEXUSPROCGEN_API FNProcGenOperationTaskGraph
{
public:
	explicit FNProcGenOperationTaskGraph(UNProcGenOperation* Generator, FNProcGenOperationContext* Context);
	
	void UnlockTasks();
	
	void WaitForTasks();
	
	void Reset();

	bool IsTasksUnlocked() const { return bTasksUnlocked; }

	
	int GetTotalPasses() const;
	int GetCompletedPasses();
	
	FIntVector2 GetTaskStatus() const;

private:
	
	TArray<FGraphEventArray> PassTasks;
	
	FGraphEventRef FinalizeTask;
	
	bool bTasksUnlocked = false;
};