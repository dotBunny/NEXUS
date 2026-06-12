// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Collections/NGameplayTagCounter.h"
#include "HAL/CriticalSection.h"
#include "Templates/Function.h"

/**
 * A thread-safe cache of data around operations.
 * @note Every public method takes OperationContextLock for its full duration, so each query or mutation is atomic
 * with respect to AddOperationContext/RemoveOperationContext. No references into the backing maps ever escape the
 * lock, so callers cannot observe a dangling entry.
 */
class NEXUSWORLDASSEMBLY_API FNWorldAssemblyContextCache
{
public:
	/**
	 * Store (or overwrite) the tag-counter and context-tag snapshot for an operation.
	 * @note Thread-safe: may be called concurrently from worker threads and the game thread.
	 */
	static void AddOperationContext(int32 OperationTicket, const FNGameplayTagCounter& TagCounter, const FGameplayTagContainer& TagContainer);

	/** @return true if a snapshot is currently cached for the operation. */
	static bool HasOperation(int32 OperationTicket);

	/**
	 * Drop the cached snapshot for an operation; a no-op if the ticket was never registered.
	 * @note Thread-safe: may be called concurrently from worker threads and the game thread.
	 */
	static void RemoveOperationContext(int32 OperationTicket);

	/**
	 * Drop the cached snapshots for every supplied operation; tickets that were never registered are skipped.
	 * @param OperationTickets The operations whose snapshots should be removed.
	 * @note Thread-safe: the whole batch is removed under a single lock acquisition.
	 */
	static void ClearContext(const TArray<int32>& OperationTickets);

	
	static void ClearAllContext();
	
	/**
	 * Read the tag-counter value for a tag without exposing the cached counter.
	 * @param OperationTicket The operation whose counter to query.
	 * @param Tag The tag to read.
	 * @param OutCount Receives the count when the operation and tag are both present; left unchanged otherwise.
	 * @return true if the operation and tag were found and OutCount was written; false otherwise.
	 */
	static bool TryGetTagCount(int32 OperationTicket, const FGameplayTag& Tag, int32& OutCount);

	/** @return true if the operation is cached and its counter tracks the tag. */
	static bool HasTagCount(int32 OperationTicket, const FGameplayTag& Tag);

	/** Add a value to the operation's count for a tag; a no-op if the operation is not cached. */
	static void AddTagCount(int32 OperationTicket, const FGameplayTag& Tag, int32 Value);

	/** Subtract a value from the operation's count for a tag; a no-op if the operation is not cached. */
	static void SubtractTagCount(int32 OperationTicket, const FGameplayTag& Tag, int32 Value);

	/** @return true if the operation is cached and its context tags contain all of TagContainer exactly. */
	static bool HasContextTags(int32 OperationTicket, const FGameplayTagContainer& TagContainer);

	/** Append tags to the operation's context tags; a no-op if the operation is not cached. */
	static void AppendContextTags(int32 OperationTicket, const FGameplayTagContainer& TagContainer);

	/** Remove tags from the operation's context tags; a no-op if the operation is not cached. */
	static void RemoveContextTags(int32 OperationTicket, const FGameplayTagContainer& TagContainer);

private:
	static TMap<int32, FNGameplayTagCounter> OperationGameplayTagCounters;
	static TMap<int32, FGameplayTagContainer> OperationGameplayContextTags;

	/**
	 * Guards both operation maps. A single lock covers the pair so each ticket's two entries are mutated atomically.
	 * @note Every accessor of the maps above MUST hold this lock for the full duration of its access, and no method
	 * may return a reference or pointer into either map — TMap rehashing on Add/Remove would leave it dangling.
	 */
	static FCriticalSection OperationContextLock;
};
