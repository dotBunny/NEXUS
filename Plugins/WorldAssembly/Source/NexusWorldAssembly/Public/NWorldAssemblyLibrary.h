// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NWorldAssemblyContextCache.h"
#include "Cell/NCellLevelInstance.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/NSeedGenerator.h"
#include "NWorldAssemblyLibrary.generated.h"

/**
 * A utility class providing functionality to support World Assembly operations.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | World Assembly")
class UNWorldAssemblyLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** @return A freshly generated human-friendly seed string suitable for use as FNAssemblyOperationSettings::Seed. */
	UFUNCTION(BlueprintPure, Category = "NEXUS|WorldAssembly", DisplayName="Get New Friendly Seed")
	static FString GetNewFriendlySeed() { return FNSeedGenerator::RandomFriendlySeed(); }
	
	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The final context tags associated with the world assembly.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Context Tags")
	static FGameplayTagContainer& GetContextTags(ANCellLevelInstance* LevelInstance)
	{
		return LevelInstance->GetContextTags();
	}
	
	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The state of context tags when this cell was selected.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Context Tags State")
	static FGameplayTagContainer& GetContextTagsState(ANCellLevelInstance* LevelInstance)
	{
		return LevelInstance->GetContextTagsState();
	}
	
	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The context tags this cell added to the overall state.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Context Tags Added")
	static FGameplayTagContainer& GetContextTagsAdded(ANCellLevelInstance* LevelInstance)
	{
		return LevelInstance->GetContextTagsAdded();
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The assembly tags used by the cell itself.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Assembly Tags")
	static FGameplayTagContainer& GetAssemblyTags(ANCellLevelInstance* LevelInstance)
	{
		return LevelInstance->GetAssemblyTags();
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The cell's seed formatted as a human-readable hexadecimal string.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Hex Seed")
	static FString GetHexSeed(ANCellLevelInstance* LevelInstance)
	{
		return FNSeedGenerator::HexFromSeed(LevelInstance->GetSeed());
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The identifier of the graph node this cell was assembled from.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Node Identifier")
	static int32 GetNodeIdentifier(ANCellLevelInstance* LevelInstance)
	{
		return LevelInstance->GetNodeIdentifier();
	}
	
	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The final TagCounter for the assembly operation.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Tag Counter (Map)")
	static TMap<FGameplayTag, int32> GetTagCounter(ANCellLevelInstance* LevelInstance)
	{
		return LevelInstance->GetTagCounter();
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The final TagCounter for the assembly operation as an array of tag/count pairs.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Tag Counter (Array)")
	static TArray<FNGameplayTagCount> GetTagCounterArray(ANCellLevelInstance* LevelInstance)
	{
		return LevelInstance->GetTagCounterArray();
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The state of the TagCounter when this cell was added, as an array of tag/count pairs.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Tag Counter State (Array)")
	static TArray<FNGameplayTagCount> GetTagCounterStateArray(ANCellLevelInstance* LevelInstance)
	{
		return LevelInstance->GetTagCounterStateArray();
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The ticket identifying the assembly operation this cell belongs to, used to key the context cache.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Operation Ticket")
	static int32 GetOperationTicket(ANCellLevelInstance* LevelInstance)
	{
		return LevelInstance->GetOperationTicket();
	}
	
	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The state of the TagCounter when this cell was added.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Tag Counter State (Map)")
	static TMap<FGameplayTag, int32> GetTagCounterState(ANCellLevelInstance* LevelInstance)
	{
		return LevelInstance->GetTagCounterState();
	}
	
	
	/**
	 * @param LevelInstance The cell level instance to query.
	 * @param Tag Tag to look for.
	 * @return true if the cell's final TagCounter contains an entry for Tag.
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Has Tag Counter")
	static bool HasTagCounter(ANCellLevelInstance* LevelInstance, const FGameplayTag Tag)
	{
		return LevelInstance->GetTagCounter().Contains(Tag);
	};

	/**
	 * Exec-pin variant of Has Tag Counter; the result drives the True/False execution outputs in Blueprint.
	 * @param LevelInstance The cell level instance to query.
	 * @param Tag Tag to look for.
	 * @return true if the cell's final TagCounter contains an entry for Tag.
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Has Tag Counter ?",
		meta = (ExpandBoolAsExecs="ReturnValue"))
	static bool HasTagCounterExec(ANCellLevelInstance* LevelInstance, const FGameplayTag Tag)
	{
		return LevelInstance->GetTagCounter().Contains(Tag);
	};
	
	/**
	 * Tests whether the context cache still holds state for an assembly operation. The cache is keyed by the
	 * operation's ticket (see Get Operation Ticket) and is what the Operation Tag Counter / Context Tag accessors below
	 * read and mutate.
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @return true if cached context state exists for OperationTicket.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Has Operation Context Cache")
	static bool HasContextCache(int32 OperationTicket)
	{
		return FNWorldAssemblyContextCache::HasOperation(OperationTicket);
	}
	
	/**
	 * Exec-pin variant of Has Operation Context Cache; the result drives the True/False execution outputs in Blueprint.
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @return true if cached context state exists for OperationTicket.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Has Operation Context Cache ?",
		meta = (ExpandBoolAsExecs="ReturnValue"))
	static bool HasContextCacheExec(int32 OperationTicket)
	{
		return FNWorldAssemblyContextCache::HasOperation(OperationTicket);
	}

	/**
	 * Reads the cached operation's running tag counter for a single tag.
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param Tag Tag whose counter value to read.
	 * @return The counter value, or -1 if the operation or tag is not present in the cache.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Operation Tag Counter")
	static int32 GetOperationTagCounter(int32 OperationTicket, FGameplayTag Tag)
	{
		int32 Count = -1;
		FNWorldAssemblyContextCache::TryGetTagCount(OperationTicket, Tag, Count);
		return Count;
	}

	/**
	 * Reads the cached operation's running tag counter for a single tag, distinguishing "absent" from a real value (unlike
	 * Get Operation Tag Counter, which collapses both to -1).
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param Tag Tag whose counter value to read.
	 * @param OutValue Receives the counter value when found; left unchanged otherwise.
	 * @return true if a counter entry exists for Tag on this operation.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="TryGet Operation Tag Counter")
	static bool TryGetOperationTagCounter(int32 OperationTicket, FGameplayTag Tag, int32& OutValue)
	{
		return FNWorldAssemblyContextCache::TryGetTagCount(OperationTicket, Tag, OutValue);
	}

	/**
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param Tag Tag to look for.
	 * @return true if the cached operation has a counter entry for Tag.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Has Operation Tag Counter")
	static bool HasOperationTagCounter(int32 OperationTicket, FGameplayTag Tag)
	{
		return FNWorldAssemblyContextCache::HasTagCount(OperationTicket, Tag);
	}

	/**
	 * Adds to the cached operation's running counter for a tag, creating the entry if needed.
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param Tag Tag whose counter to increase.
	 * @param Value Amount to add (defaults to 1).
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Add Operation Tag Counter")
	static void AddOperationTagCounter(int32 OperationTicket, FGameplayTag Tag, int32 Value = 1)
	{
		FNWorldAssemblyContextCache::AddTagCount(OperationTicket, Tag, Value);
	}

	/**
	 * Subtracts from the cached operation's running counter for a tag.
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param Tag Tag whose counter to decrease.
	 * @param Value Amount to subtract (defaults to 1).
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Subtract Operation Tag Counter")
	static void SubtractOperationTagCounter(int32 OperationTicket, FGameplayTag Tag, int32 Value = 1)
	{
		FNWorldAssemblyContextCache::SubtractTagCount(OperationTicket, Tag, Value);
	}

	/**
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param TagContainer Tags that must all be present.
	 * @return true if the cached operation's context tags contain every tag in TagContainer (exact match).
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Has Operation Context Tag(s)")
	static bool HasOperationContextTags(int32 OperationTicket, FGameplayTagContainer TagContainer)
	{
		return FNWorldAssemblyContextCache::HasContextTags(OperationTicket, TagContainer);
	}
	
	/**
	 * Exec-pin variant of Has Operation Context Tag(s); the result drives the True/False execution outputs in Blueprint.
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param TagContainer Tags that must all be present.
	 * @return true if the cached operation's context tags contain every tag in TagContainer (exact match).
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Has Operation Context Tag(s) ?",
		meta = (ExpandBoolAsExecs="ReturnValue"))
	static bool HasOperationContextTagsExec(int32 OperationTicket, FGameplayTagContainer TagContainer)
	{
		return FNWorldAssemblyContextCache::HasContextTags(OperationTicket, TagContainer);
	}

	/**
	 * Adds tags to the cached operation's context tag set (union; duplicates are ignored).
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param TagContainer Tags to add.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Append Operation Context Tag(s)")
	static void AppendOperationContextTags(int32 OperationTicket, FGameplayTagContainer TagContainer)
	{
		FNWorldAssemblyContextCache::AppendContextTags(OperationTicket, TagContainer);
	}

	/**
	 * Removes tags from the cached operation's context tag set.
	 * @param OperationTicket Ticket identifying the assembly operation.
	 * @param TagContainer Tags to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Remove Operation Context Tag(s)")
	static void RemoveOperationContextTags(int32 OperationTicket, FGameplayTagContainer TagContainer)
	{
		FNWorldAssemblyContextCache::RemoveContextTags(OperationTicket, TagContainer);
	}
};