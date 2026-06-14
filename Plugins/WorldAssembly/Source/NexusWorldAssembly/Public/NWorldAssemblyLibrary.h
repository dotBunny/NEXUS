// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NWorldAssemblyContextCache.h"
#include "NWorldAssemblyMinimal.h"
#include "NWorldAssemblySettings.h"
#include "Cell/NCellJunctionComponent.h"
#include "Cell/NCellLevelInstance.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/NSeedGenerator.h"
#include "NWorldAssemblyLibrary.generated.h"

/**
 * A utility class providing functionality to support World Assembly operations.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | World Assembly")
class NEXUSWORLDASSEMBLY_API UNWorldAssemblyLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName = "Get Junction World Size")
	static FVector GetJunctionWorldSize(UNCellJunctionComponent* JunctionComponent, bool bWithDepth = false)
	{
		if (JunctionComponent == nullptr)
		{
			UE_LOG(LogNexusWorldAssembly, Error, TEXT("Unable to get Junction World Size as no JunctionComponent was made available."));
			return  FVector::ZeroVector;
		}
		const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get();
		FVector ReturnVector = FVector::OneVector;
		ReturnVector.X = JunctionComponent->Details.SocketSize.X * Settings->SocketSize.X;
		ReturnVector.Y = JunctionComponent->Details.SocketSize.Y * Settings->SocketSize.Y;
		if (bWithDepth)
		{
			ReturnVector.Z = Settings->SocketDepth;
		}
		return ReturnVector;
	}
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName = "Get Junction World Size (Shifted)", meta=(ToolTip="Depth, X, Y"))
	static FVector GetJunctionWorldSizeShifted(UNCellJunctionComponent* JunctionComponent, float Scale = 1.f)
	{
		if (JunctionComponent == nullptr)
		{
			UE_LOG(LogNexusWorldAssembly, Error, TEXT("Unable to get Junction World Size as no JunctionComponent was made available."));
			return  FVector::ZeroVector;
		}
		const UNWorldAssemblySettings* Settings = UNWorldAssemblySettings::Get();
		return FVector(
			Settings->SocketDepth * Scale, 
			(JunctionComponent->Details.SocketSize.X * Settings->SocketSize.X) * Scale, 
			(JunctionComponent->Details.SocketSize.Y * Settings->SocketSize.Y) * Scale);
	}
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName = "Get Junction World Corner Points")
	static TArray<FVector> GetJunctionWorldCornerPoints(UNCellJunctionComponent* JunctionComponent, const FVector2D& SocketSize)
	{
		if (JunctionComponent == nullptr)
		{
			UE_LOG(LogNexusWorldAssembly, Error, TEXT("Unable to get Junction World Corner Points as no JunctionComponent was made available."));
			return  TArray<FVector>();
		}
		return JunctionComponent->GetWorldCornerPoints(SocketSize);
	};
	
	
	/** @return A freshly generated human-friendly seed string suitable for use as FNAssemblyOperationSettings::Seed. */
	UFUNCTION(BlueprintPure, Category = "NEXUS|WorldAssembly", DisplayName="Get New Friendly Seed")
	static FString GetNewFriendlySeed() { return FNSeedGenerator::RandomFriendlySeed(); }
	
	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The final context tags associated with the world assembly.
	 * @note Returns a copy; edits made to it are not written back to the cell. Use Append/Remove Context Tags to persist changes.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Context Tags")
	static FGameplayTagContainer GetContextTags(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) ? LevelInstance->GetContextTags() : FGameplayTagContainer();
	}
	
	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The context tags this cell added to the overall state.
	 * @note Returns a copy; edits made to it are not written back to the cell.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Context Tags Added")
	static FGameplayTagContainer GetContextTagsAdded(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) ? LevelInstance->GetContextTagsAdded() : FGameplayTagContainer();
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The assembly tags used by the cell itself.
	 * @note Returns a copy; edits made to it are not written back to the cell.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Assembly Tags")
	static FGameplayTagContainer GetAssemblyTags(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) ? LevelInstance->GetAssemblyTags() : FGameplayTagContainer();
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The cell's seed formatted as a human-readable hexadecimal string.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Hex Seed")
	static FString GetHexSeed(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) ? FNSeedGenerator::HexFromSeed(LevelInstance->GetSeed()) : FString();
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The identifier of the graph node this cell was assembled from.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Node Identifier")
	static int32 GetNodeIdentifier(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) ? LevelInstance->GetNodeIdentifier() : INDEX_NONE;
	}
	
	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The final TagCounter for the assembly operation.
	 * @note Returns a copy; edits made to it are not written back to the cell. Use Add/Subtract Tag Counter to persist changes.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Tag Counter (Map)")
	static TMap<FGameplayTag, int32> GetTagCounter(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) ? LevelInstance->GetTagCounter() : TMap<FGameplayTag, int32>();
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The final TagCounter for the assembly operation as an array of tag/count pairs.
	 * @note Returns a copy; edits made to it are not written back to the cell. Use Add/Subtract Tag Counter to persist changes.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Tag Counter (Array)")
	static TArray<FNGameplayTagCount> GetTagCounterArray(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) ? LevelInstance->GetTagCounterArray() : TArray<FNGameplayTagCount>();
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The ticket identifying the assembly operation this cell belongs to, used to key the context cache.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Operation Ticket")
	static int32 GetOperationTicket(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) ? LevelInstance->GetOperationTicket() : INDEX_NONE;
	}

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return true if this cell lies on the assembly's hot path.
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Is HotPath")
	static bool IsHotPath(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) && LevelInstance->IsHotPath();
	};

	/**
	 * Exec-pin variant of Is HotPath; the result drives the True/False execution outputs in Blueprint.
	 * @param LevelInstance The cell level instance to query.
	 * @return true if this cell lies on the assembly's hot path.
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Is HotPath ?",
		meta = (ExpandBoolAsExecs="ReturnValue"))
	static bool IsHotPathExec(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) && LevelInstance->IsHotPath();
	};

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return true if this cell lies on the shortest-path hot path (spokes from the start cell).
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Is HotPath (Shortest)")
	static bool IsHotPathShortest(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) && LevelInstance->IsHotPathShortest();
	};
	
	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return true if this cell lies on the shortest-path hot path (spokes from the start cell).
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Is HotPath (Shortest) ?",
		meta = (ExpandBoolAsExecs="ReturnValue"))
	static bool IsHotPathShortestExec(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) && LevelInstance->IsHotPathShortest();
	};

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return true if this cell lies on the sequential hot path (nearest-first visiting chain).
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Is HotPath (Sequential)")
	static bool IsHotPathSequential(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) && LevelInstance->IsHotPathSequential();
	};
	
	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return true if this cell lies on the sequential hot path (nearest-first visiting chain).
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Is HotPath (Sequential) ?",
		meta = (ExpandBoolAsExecs="ReturnValue"))
	static bool IsHotPathSequentialExec(ANCellLevelInstance* LevelInstance)
	{
		return IsValid(LevelInstance) && LevelInstance->IsHotPathSequential();
	};
	
	/**
	 * @param LevelInstance The cell level instance to query.
	 * @param Tag Tag to look for.
	 * @return true if the cell's final TagCounter contains an entry for Tag.
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Has Tag Counter")
	static bool HasTagCounter(ANCellLevelInstance* LevelInstance, const FGameplayTag Tag)
	{
		return IsValid(LevelInstance) && LevelInstance->GetTagCounter().Contains(Tag);
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
		return IsValid(LevelInstance) && LevelInstance->GetTagCounter().Contains(Tag);
	};

	/**
	 * @param LevelInstance The cell level instance to query.
	 * @param TagContainer Tags that must all be present.
	 * @return true if the cell's final context tags contain every tag in TagContainer (exact match).
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Has Context Tag(s)")
	static bool HasContextTags(ANCellLevelInstance* LevelInstance, const FGameplayTagContainer TagContainer)
	{
		return IsValid(LevelInstance) && LevelInstance->GetContextTags().HasAllExact(TagContainer);
	};

	/**
	 * Exec-pin variant of Has Context Tag(s); the result drives the True/False execution outputs in Blueprint.
	 * @param LevelInstance The cell level instance to query.
	 * @param TagContainer Tags that must all be present.
	 * @return true if the cell's final context tags contain every tag in TagContainer (exact match).
	 */
	UFUNCTION(BlueprintCallable,  Category = "NEXUS|WorldAssembly", DisplayName = "Has Context Tag(s) ?",
		meta = (ExpandBoolAsExecs="ReturnValue"))
	static bool HasContextTagsExec(ANCellLevelInstance* LevelInstance, const FGameplayTagContainer TagContainer)
	{
		return IsValid(LevelInstance) && LevelInstance->GetContextTags().HasAllExact(TagContainer);
	};

	/**
	 * Adds tags to the cell's final context tag set (union; duplicates are ignored). Use this to persist edits:
	 * Get Context Tags returns a Blueprint copy, so changes made to that copy are not written back to the cell.
	 * @param LevelInstance The cell level instance to modify.
	 * @param TagContainer Tags to add.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Append Context Tags")
	static void AppendContextTags(ANCellLevelInstance* LevelInstance, FGameplayTagContainer TagContainer)
	{
		if (IsValid(LevelInstance))
		{
			LevelInstance->GetContextTags().AppendTags(TagContainer);
		}
	}

	/**
	 * Removes tags from the cell's final context tag set.
	 * @param LevelInstance The cell level instance to modify.
	 * @param TagContainer Tags to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Remove Context Tags")
	static void RemoveContextTags(ANCellLevelInstance* LevelInstance, FGameplayTagContainer TagContainer)
	{
		if (IsValid(LevelInstance))
		{
			LevelInstance->GetContextTags().RemoveTags(TagContainer);
		}
	}

	/**
	 * Adds to the cell's final TagCounter for a tag, creating the entry if needed. Use this to persist edits:
	 * the Get Tag Counter accessors return Blueprint copies, so changes made to those copies are not written back.
	 * @param LevelInstance The cell level instance to modify.
	 * @param Tag Tag whose counter to increase.
	 * @param Value Amount to add (defaults to 1).
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Add Tag Counter")
	static void AddTagCounter(ANCellLevelInstance* LevelInstance, FGameplayTag Tag, int32 Value = 1)
	{
		if (IsValid(LevelInstance))
		{
			FindOrAddTagCount(LevelInstance->GetTagCounterArray(), Tag).Count += Value;
		}
	}

	/**
	 * Subtracts from the cell's final TagCounter for a tag, creating the entry if needed.
	 * @param LevelInstance The cell level instance to modify.
	 * @param Tag Tag whose counter to decrease.
	 * @param Value Amount to subtract (defaults to 1).
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Subtract Tag Counter")
	static void SubtractTagCounter(ANCellLevelInstance* LevelInstance, FGameplayTag Tag, int32 Value = 1)
	{
		if (IsValid(LevelInstance))
		{
			FindOrAddTagCount(LevelInstance->GetTagCounterArray(), Tag).Count -= Value;
		}
	}

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

private:
	/**
	 * Finds the counter entry for Tag in Counter, appending a zero-initialized entry (Count = 0) when absent so
	 * callers can adjust Count in place.
	 * @param Counter The flat tag-counter array to search.
	 * @param Tag The tag whose entry to locate or create.
	 * @return A reference to the existing or newly appended entry for Tag.
	 */
	static FNGameplayTagCount& FindOrAddTagCount(TArray<FNGameplayTagCount>& Counter, const FGameplayTag& Tag)
	{
		for (FNGameplayTagCount& Entry : Counter)
		{
			if (Entry.Tag == Tag)
			{
				return Entry;
			}
		}
		FNGameplayTagCount& Added = Counter.AddDefaulted_GetRef();
		Added.Tag = Tag;
		return Added;
	}
};