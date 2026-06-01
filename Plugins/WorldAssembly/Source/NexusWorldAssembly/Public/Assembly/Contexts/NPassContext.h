// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/Graph/NAssemblyGraph.h"

/**
 * Shared collect-pass context — fans-in completed organ graphs from every parallel builder task
 * so the subsequent collect/finalize stages have a single place to walk.
 */
class FNPassContext
{
public:
	/** Transfer ownership of a completed graph /tags into this pass context. */
	void TakeOutput(TUniquePtr<FNAssemblyGraph> Graph, const FGameplayTagContainer& NewOutputTags, const FGameplayTagContainer& NewContextTags)
	{
		FScopeLock Lock(&TakeOutputMutex);
		
		Graphs.Add(MoveTemp(Graph));
		OutputTags.AppendTags(NewOutputTags);
		ContextTags.AppendTags(NewContextTags);
	}

	/** Graphs accumulated from this collect pass. */
	TArray<TUniquePtr<FNAssemblyGraph>> Graphs;
	FGameplayTagContainer OutputTags;
	FGameplayTagContainer ContextTags;

private:
	FCriticalSection TakeOutputMutex;
};
