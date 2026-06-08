// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Assembly/NAssemblyOperationSettings.h"
#include "Cell/NCellProxy.h"
#include "Assembly/Graph/NAssemblyGraph.h"

/**
 * A drained snapshot of a single status channel, handed to the game thread by ConsumeChannelUpdates.
 *
 * Carries the full current state of a channel that changed since the last drain; the consumer
 * decides whether the channel is new (first time the id is seen) or an update to an existing one.
 */
struct FNStatusChannelUpdate
{
	/** Stable identifier allocated by OpenStatusChannel. */
	int32 ChannelId = INDEX_NONE;
	/** Left-aligned label captured when the channel was opened. */
	FString Label;
	/** Most recent status message published to the channel. */
	FString Message;
	/** Most recent completion percent in the 0..1 range. */
	float Percent = 0.f;
	/** true once the channel has been closed; closed channels linger at their final state. */
	bool bClosed = false;
};

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
		FScopeLock Lock(&ContextTagMutex);
		ContextTags.AppendTags(NewContextTags);
	}
	
	void AddTagCounter(const FNGameplayTagCounter& NewTagCounter)
	{
		FScopeLock Lock(&TagCounterMutex);
		TagCounter.Combine(NewTagCounter);
	}
	
	void SetOrganCellCount(const FGuid& OrganIdentifier, const int32 NewCount)
	{
		FScopeLock Lock(&OrganCellMutex);
		OrganCellCount.FindOrAdd(OrganIdentifier, NewCount);
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

	/**
	 * Open a new status channel for fine-grained per-stage progress display. Safe to call from any task thread.
	 * Each channel is surfaced as a child progress bar beneath the operation's row in the developer overlay.
	 * @param Label The left-aligned label shown for this channel's progress bar (e.g. an organ name).
	 * @return A stable identifier used to address the channel in SetChannelStatus / CloseStatusChannel.
	 */
	int32 OpenStatusChannel(const FString& Label)
	{
		const int32 ChannelId = NextChannelId.IncrementExchange();
		{
			FScopeLock Lock(&StatusChannelMutex);
			FStatusChannelState& State = StatusChannels.Add(ChannelId);
			State.Label = Label;
			State.bDirty = true;
		}
		StatusChannelVersion.IncrementExchange();
		return ChannelId;
	}

	/**
	 * Publish a message and completion percent to a channel. Safe to call from any task thread.
	 * Coalesced (latest wins) and drained on the game thread via ConsumeChannelUpdates.
	 * @param ChannelId Identifier returned by OpenStatusChannel.
	 * @param Message The new status message for this channel.
	 * @param Percent Completion percent in the 0..1 range.
	 */
	void SetChannelStatus(const int32 ChannelId, const FString& Message, const float Percent = 0.f)
	{
		{
			FScopeLock Lock(&StatusChannelMutex);
			FStatusChannelState* State = StatusChannels.Find(ChannelId);
			if (State == nullptr)
			{
				return;
			}
			State->Message = Message;
			State->Percent = FMath::Clamp(Percent, 0.f, 1.f);
			State->bDirty = true;
		}
		StatusChannelVersion.IncrementExchange();
	}

	/**
	 * Mark a channel complete. Safe to call from any task thread. The channel's bar lingers at 100%
	 * (closed channels are not removed) until the owning operation tears down.
	 * @param ChannelId Identifier returned by OpenStatusChannel.
	 */
	void CloseStatusChannel(const int32 ChannelId)
	{
		{
			FScopeLock Lock(&StatusChannelMutex);
			FStatusChannelState* State = StatusChannels.Find(ChannelId);
			if (State == nullptr)
			{
				return;
			}
			State->Percent = 1.f;
			State->bClosed = true;
			State->bDirty = true;
		}
		StatusChannelVersion.IncrementExchange();
	}

	/**
	 * Drain every channel that changed since the last call, in ascending id (open) order. Game thread only.
	 * @param OutChanges Receives the current state of each dirtied channel.
	 * @return true if any channel was drained since the last call; false if nothing changed.
	 */
	bool ConsumeChannelUpdates(TArray<FNStatusChannelUpdate>& OutChanges)
	{
		// Cheap atomic gate so we only take the lock on frames where something actually changed.
		if (StatusChannelVersion.Load() == LastConsumedStatusChannelVersion)
		{
			return false;
		}

		FScopeLock Lock(&StatusChannelMutex);

		// Ids are allocated monotonically, so sorting keys preserves a stable open order for display.
		TArray<int32> ChannelIds;
		StatusChannels.GetKeys(ChannelIds);
		ChannelIds.Sort();

		for (const int32 ChannelId : ChannelIds)
		{
			FStatusChannelState& State = StatusChannels[ChannelId];
			if (!State.bDirty)
			{
				continue;
			}
			State.bDirty = false;

			FNStatusChannelUpdate& Update = OutChanges.AddDefaulted_GetRef();
			Update.ChannelId = ChannelId;
			Update.Label = State.Label;
			Update.Message = State.Message;
			Update.Percent = State.Percent;
			Update.bClosed = State.bClosed;
		}

		LastConsumedStatusChannelVersion = StatusChannelVersion.Load();
		return OutChanges.Num() > 0;
	}

	/** Built per-organ graphs owned by this context. */
	TArray<TUniquePtr<FNAssemblyGraph>> Graphs;

	/** Context tags accumulated across the graphs built into this context. */
	FGameplayTagContainer ContextTags;
	
	FNGameplayTagCounter TagCounter;
	
	TMap<FGuid, int> OrganCellCount;

	/**
	 * @param OutputWorld World to target when spawning proxies.
	 * @param OperationTicket Identifier of the operation that owns this context.
	 * @param Settings Operation-wide settings forwarded to dependent tasks.
	 */
	explicit FNAssemblyTaskGraphContext(UWorld* OutputWorld, const int32& OperationTicket, const FNAssemblyOperationSettings& Settings);

	
	/** Path the operation's report is written to, when report output is enabled. */
	FString ReportFilePath;

private:
	FCriticalSection ContextTagMutex;
	FCriticalSection TagCounterMutex;
	FCriticalSection TakeGraphMutex;
	FCriticalSection OrganCellMutex;

	/** Guards PendingDisplayMessage against concurrent writes from task threads. */
	FCriticalSection DisplayMessageMutex;
	
	/** Most recent display message written by a task; coalesced (latest wins). */
	FString PendingDisplayMessage;
	/** Bumped on every SetDisplayMessage so the game-thread reader can cheaply detect changes without locking. */
	TAtomic<uint32> DisplayMessageVersion{0};
	/** Version last drained by ConsumeDisplayMessage; touched only on the game thread. */
	uint32 LastConsumedDisplayMessageVersion = 0;

	/** Mutable state of one status channel; coalesced (latest wins) and drained on the game thread. */
	struct FStatusChannelState
	{
		/** Left-aligned label captured when the channel was opened. */
		FString Label;
		/** Most recent status message published to the channel. */
		FString Message;
		/** Most recent completion percent in the 0..1 range. */
		float Percent = 0.f;
		/** true once CloseStatusChannel has been called for this channel. */
		bool bClosed = false;
		/** true while the channel has unread changes pending a game-thread drain. */
		bool bDirty = true;
	};

	/** Guards StatusChannels against concurrent writes from task threads. */
	FCriticalSection StatusChannelMutex;
	/** Live status channels keyed by their allocated identifier. */
	TMap<int32, FStatusChannelState> StatusChannels;
	/** Source of monotonically increasing channel identifiers; also defines stable display order. */
	TAtomic<int32> NextChannelId{0};
	/** Bumped on every channel mutation so the game-thread reader can cheaply detect changes without locking. */
	TAtomic<uint32> StatusChannelVersion{0};
	/** Version last drained by ConsumeChannelUpdates; touched only on the game thread. */
	uint32 LastConsumedStatusChannelVersion = 0;
};
