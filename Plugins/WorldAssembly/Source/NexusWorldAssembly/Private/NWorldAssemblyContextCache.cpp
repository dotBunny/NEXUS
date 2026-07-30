// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyContextCache.h"

#include "Misc/ScopeLock.h"

void FNWorldAssemblyContextCache::AddOperationContext(const int32 OperationTicket, const FNGameplayTagCounter& TagCounter, const FGameplayTagContainer& TagContainer)
{
	// TMap is not concurrency-safe, so serialize all map access behind the lock. TMap::Add inserts or overwrites,
	// which folds the previous Contains/else assignment into a single call under the critical section.
	FScopeLock Lock(&OperationContextLock);
	OperationGameplayTagCounters.Add(OperationTicket, TagCounter);
	OperationGameplayContextTags.Add(OperationTicket, TagContainer);
}

void FNWorldAssemblyContextCache::RemoveOperationContext(const int32 OperationTicket)
{
	// TMap::Remove already no-ops when the key is absent, so the Contains guards are unnecessary under the lock.
	FScopeLock Lock(&OperationContextLock);
	OperationGameplayTagCounters.Remove(OperationTicket);
	OperationGameplayContextTags.Remove(OperationTicket);
}

void FNWorldAssemblyContextCache::ClearContext(const TArray<int32>& OperationTickets)
{
	// Take the lock once for the whole batch rather than per-ticket. TMap::Remove already no-ops on absent keys.
	FScopeLock Lock(&OperationContextLock);
	for (const int32 OperationTicket : OperationTickets)
	{
		OperationGameplayTagCounters.Remove(OperationTicket);
		OperationGameplayContextTags.Remove(OperationTicket);
	}
}

void FNWorldAssemblyContextCache::ClearAllContext()
{
	FScopeLock Lock(&OperationContextLock);
	OperationGameplayContextTags.Empty();
	OperationGameplayTagCounters.Empty();
}

bool FNWorldAssemblyContextCache::HasOperation(const int32 OperationTicket)
{
	FScopeLock Lock(&OperationContextLock);
	return OperationGameplayTagCounters.Contains(OperationTicket);
}

bool FNWorldAssemblyContextCache::TryGetTagCount(const int32 OperationTicket, const FGameplayTag& Tag, int32& OutCount)
{
	FScopeLock Lock(&OperationContextLock);
	const FNGameplayTagCounter* Counter = OperationGameplayTagCounters.Find(OperationTicket);
	if (Counter == nullptr || !Counter->GameplayTags.Contains(Tag))
	{
		return false;
	}

	// Read the value out under the lock; nothing referencing the map escapes.
	OutCount = Counter->GameplayTags[Tag];
	return true;
}

bool FNWorldAssemblyContextCache::HasTagCount(const int32 OperationTicket, const FGameplayTag& Tag)
{
	FScopeLock Lock(&OperationContextLock);
	const FNGameplayTagCounter* Counter = OperationGameplayTagCounters.Find(OperationTicket);
	return Counter != nullptr && Counter->Has(Tag);
}

void FNWorldAssemblyContextCache::AddTagCount(const int32 OperationTicket, const FGameplayTag& Tag, const int32 Value)
{
	FScopeLock Lock(&OperationContextLock);
	if (FNGameplayTagCounter* Counter = OperationGameplayTagCounters.Find(OperationTicket))
	{
		Counter->Add(Tag, Value);
	}
}

void FNWorldAssemblyContextCache::SubtractTagCount(const int32 OperationTicket, const FGameplayTag& Tag, const int32 Value)
{
	FScopeLock Lock(&OperationContextLock);
	if (FNGameplayTagCounter* Counter = OperationGameplayTagCounters.Find(OperationTicket))
	{
		Counter->Subtract(Tag, Value);
	}
}

bool FNWorldAssemblyContextCache::HasContextTags(const int32 OperationTicket, const FGameplayTagContainer& TagContainer)
{
	FScopeLock Lock(&OperationContextLock);
	const FGameplayTagContainer* ContextTags = OperationGameplayContextTags.Find(OperationTicket);
	return ContextTags != nullptr && ContextTags->HasAllExact(TagContainer);
}

void FNWorldAssemblyContextCache::AppendContextTags(const int32 OperationTicket, const FGameplayTagContainer& TagContainer)
{
	FScopeLock Lock(&OperationContextLock);
	if (FGameplayTagContainer* ContextTags = OperationGameplayContextTags.Find(OperationTicket))
	{
		ContextTags->AppendTags(TagContainer);
	}
}

void FNWorldAssemblyContextCache::RemoveContextTags(const int32 OperationTicket, const FGameplayTagContainer& TagContainer)
{
	FScopeLock Lock(&OperationContextLock);
	if (FGameplayTagContainer* ContextTags = OperationGameplayContextTags.Find(OperationTicket))
	{
		ContextTags->RemoveTags(TagContainer);
	}
}

TMap<int32, FNGameplayTagCounter> FNWorldAssemblyContextCache::OperationGameplayTagCounters;
TMap<int32, FGameplayTagContainer> FNWorldAssemblyContextCache::OperationGameplayContextTags;
FCriticalSection FNWorldAssemblyContextCache::OperationContextLock;
