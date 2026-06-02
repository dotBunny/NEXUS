// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Cell/NCellLevelInstance.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/NSeedGenerator.h"
#include "NWorldAssemblyLibrary.generated.h"

/**
 * A utility class providing functionality support World Assembly operations.
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
	

	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Tag Counter (Array)")
	static TArray<FNGameplayTagCount> GetTagCounterArray(ANCellLevelInstance* LevelInstance)
	{
		return LevelInstance->GetTagCounterArray();
	}
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Tag Counter State (Array)")
	static TArray<FNGameplayTagCount> GetTagCounterStateArray(ANCellLevelInstance* LevelInstance)
	{
		return LevelInstance->GetTagCounterStateArray();
	}
	
	/**
	 * @param LevelInstance The cell level instance to query.
	 * @return The state of the TagCounter when this cell was addded.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Tag Counter State (Map)")
	static TMap<FGameplayTag, int32> GetTagCounterState(ANCellLevelInstance* LevelInstance)
	{
		return LevelInstance->GetTagCounterState();
	}
};