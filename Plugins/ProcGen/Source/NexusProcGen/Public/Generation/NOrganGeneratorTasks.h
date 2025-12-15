// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class UNProcGenOperation;
class FNOrganGenerationContext;

class NEXUSPROCGEN_API FNOrganGeneratorTasks
{
public:
	explicit FNOrganGeneratorTasks(UNProcGenOperation* Generator, FNOrganGenerationContext* Context);
	
	void UnlockTasks();
	
	void WaitForTasks();
	
	void Reset();

	bool IsTasksUnlocked() const { return bTasksUnlocked; }

	
	int GetTotalPasses() const;
	
	int GetCompletedPasses();
	
	
private:

	TArray<FGraphEventArray> PassTasks;
	FGraphEventRef FinalizeTask;
	
	bool bTasksUnlocked = false;
};