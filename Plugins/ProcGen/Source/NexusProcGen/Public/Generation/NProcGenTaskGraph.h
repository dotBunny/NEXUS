// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"

class UNProcGenOperation;
class FNProcGenOperationContext;

class NEXUSPROCGEN_API FNProcGenTaskGraph
{
public:
	/**
	 * Creates the executable task-graph based on the provided Operation and initial Context.
	 * @param Operation The instigating generation operation which will own this graph and its execution.
	 * @param Context The world/generation context to use when building out the graph.
	 */
	explicit FNProcGenTaskGraph(UNProcGenOperation* Operation, FNProcGenOperationContext* Context);
	
	FIntVector2 GetTaskStatus() const;
	bool IsTasksUnlocked() const { return bTasksUnlocked; }
	void ResetGraph();
	void UnlockTasks();
	void WaitForTasks();

private:
	bool bTasksUnlocked = false;
	
	FGraphEventRef FinalizeTask;
	FGraphEventArray AllTasks;
	TArray<FGraphEventArray> GraphBuilderTasks;
	FGraphEventArray CollectionTasks;
	FGraphEventArray FinalizerTasks;
	
};