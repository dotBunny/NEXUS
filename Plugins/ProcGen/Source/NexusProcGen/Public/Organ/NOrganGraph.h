// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class FNOrganGeneratorContext;

class NEXUSPROCGEN_API FNOrganGraph
{
public:
	explicit FNOrganGraph(FNOrganGeneratorContext* Context);
	
	void UnlockTasks();
	
	void WaitForTasks();
	
	void Reset();

	bool IsTasksUnlocked() const { return bTasksUnlocked; }
	
private:

	TArray<FGraphEventArray> PassTasks;
	FGraphEventRef FinalizeTask;
	
	bool bTasksUnlocked = false;
};