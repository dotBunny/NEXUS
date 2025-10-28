// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class FNOrganContext;

class NEXUSPROCGEN_API FNOrganGraph
{
public:
	explicit FNOrganGraph(FNOrganContext* Context);
	
	void UnlockTasks();
	
	void WaitForTasks();
	
	void Reset();

	bool IsTasksUnlocked() const { return bTasksUnlocked; }
	
private:
	FNOrganContext* RootContext;
	TArray<FGraphEventArray> PassTasks;
	FGraphEventRef FinalizeTask;
	
	bool bTasksUnlocked = false;
};