// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/NAssemblyOperationSettings.h"
#include "Cell/NCellProxy.h"
#include "Assembly/Graph/NAssemblyGraph.h"

/**
 * Execution-time context shared across all tasks in a single FNAssemblyTaskGraph run.
 *
 * Collects the graph results (per-organ graphs, spawned proxies) plus the settings/ticket
 * each task needs to operate without reaching back into the operation.
 */
class FNAssemblyTaskGraphContext
{
public:
	/** The UWorld to target when creating or querying from the FNAssemblyTaskGraph. */
	TObjectPtr<UWorld> TargetWorld;

	/** The proxies that were created from the evaluated task graph. */
	TArray<TObjectPtr<ANCellProxy>> CreatedProxies;

	/** The operation identifier for this graph. */
	int32 OperationTicket;

	/** Operation-specific settings effecting the behavior of some bits of the graph. */
	FNAssemblyOperationSettings OperationSettings;

	/** Transfer ownership of a built organ-graph into this context. */
	void TakeGraph(TUniquePtr<FNAssemblyGraph> Graph)
	{
		FScopeLock Lock(&TakeGraphMutex);
		Graphs.Add(MoveTemp(Graph));
	}

	/** Append additional context tags to this context's accumulated set. */
	void AddContextTags(const FGameplayTagContainer& NewContextTags)
	{
		ContextTags.AppendTags(NewContextTags);
	}
	
	void AddTagCounter(const FNGameplayTagCounter& NewTagCounter)
	{
		TagCounter.Combine(NewTagCounter);
	}

	/**
	 * Record a progress/status message for display. Safe to call from any task thread.
	 * The message is coalesced (latest wins) and drained on the game thread via ConsumeDisplayMessage.
	 * @param Message The new display message.
	 */
	void SetStatusMessage(const FString& Message)
	{
		FScopeLock Lock(&DisplayMessageMutex);
		PendingDisplayMessage = Message;
		DisplayMessageVersion.IncrementExchange();
	}

	/**
	 * Drain the most recent display message. Game thread only.
	 * @param OutMessage Receives the pending message when one is available.
	 * @return true if a new message was drained since the last call; false if nothing changed.
	 */
	bool ConsumeDisplayMessage(FString& OutMessage)
	{
		// Cheap atomic gate so we only take the lock on frames where something actually changed.
		if (DisplayMessageVersion.Load() == LastConsumedDisplayMessageVersion)
		{
			return false;
		}
		FScopeLock Lock(&DisplayMessageMutex);
		OutMessage = PendingDisplayMessage;
		LastConsumedDisplayMessageVersion = DisplayMessageVersion.Load();
		return true;
	}

	/** Built per-organ graphs owned by this context. */
	TArray<TUniquePtr<FNAssemblyGraph>> Graphs;

	/** Context tags accumulated across the graphs built into this context. */
	FGameplayTagContainer ContextTags;
	
	FNGameplayTagCounter TagCounter;

	/**
	 * @param OutputWorld World to target when spawning proxies.
	 * @param OperationTicket Identifier of the operation that owns this context.
	 * @param Settings Operation-wide settings forwarded to dependent tasks.
	 */
	explicit FNAssemblyTaskGraphContext(UWorld* OutputWorld, const int32& OperationTicket, const FNAssemblyOperationSettings& Settings);

	
	/** Path the operation's report is written to, when report output is enabled. */
	FString ReportFilePath;

private:
	FCriticalSection TakeGraphMutex;

	/** Guards PendingDisplayMessage against concurrent writes from task threads. */
	FCriticalSection DisplayMessageMutex;
	/** Most recent display message written by a task; coalesced (latest wins). */
	FString PendingDisplayMessage;
	/** Bumped on every SetDisplayMessage so the game-thread reader can cheaply detect changes without locking. */
	TAtomic<uint32> DisplayMessageVersion{0};
	/** Version last drained by ConsumeDisplayMessage; touched only on the game thread. */
	uint32 LastConsumedDisplayMessageVersion = 0;
};
