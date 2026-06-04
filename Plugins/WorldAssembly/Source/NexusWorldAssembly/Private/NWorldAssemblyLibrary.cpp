// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyLibrary.h"

#include "NWorldAssemblyContextCache.h"

bool UNWorldAssemblyLibrary::HasContextCache(const int32 OperationTicket)
{
	return FNWorldAssemblyContextCache::HasOperation(OperationTicket);
}

int32 UNWorldAssemblyLibrary::GetOperationTagCounter(const int32 OperationTicket, const FGameplayTag Tag)
{
	int32 Count = -1;
	FNWorldAssemblyContextCache::TryGetTagCount(OperationTicket, Tag, Count);
	return Count;
}

bool UNWorldAssemblyLibrary::TryGetOperationTagCounter(const int32 OperationTicket, const FGameplayTag Tag, int32& OutValue)
{
	return FNWorldAssemblyContextCache::TryGetTagCount(OperationTicket, Tag, OutValue);
}

bool UNWorldAssemblyLibrary::HasOperationTagCounter(const int32 OperationTicket, const FGameplayTag Tag)
{
	return FNWorldAssemblyContextCache::HasTagCount(OperationTicket, Tag);
}

void UNWorldAssemblyLibrary::AddOperationTagCounter(const int32 OperationTicket, const FGameplayTag Tag, const int32 Value)
{
	FNWorldAssemblyContextCache::AddTagCount(OperationTicket, Tag, Value);
}

void UNWorldAssemblyLibrary::SubtractOperationTagCounter(const int32 OperationTicket, const FGameplayTag Tag, const int32 Value)
{
	FNWorldAssemblyContextCache::SubtractTagCount(OperationTicket, Tag, Value);
}

bool UNWorldAssemblyLibrary::HasOperationContextTags(const int32 OperationTicket, const FGameplayTagContainer TagContainer)
{
	return FNWorldAssemblyContextCache::HasContextTags(OperationTicket, TagContainer);
}

void UNWorldAssemblyLibrary::AppendOperationContextTags(const int32 OperationTicket, const FGameplayTagContainer TagContainer)
{
	FNWorldAssemblyContextCache::AppendContextTags(OperationTicket, TagContainer);
}

void UNWorldAssemblyLibrary::RemoveOperationContextTags(const int32 OperationTicket, const FGameplayTagContainer TagContainer)
{
	FNWorldAssemblyContextCache::RemoveContextTags(OperationTicket, TagContainer);
}