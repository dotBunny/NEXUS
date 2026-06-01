// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Cell/NCellActor.h"
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
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Context Tags (ANCellLevelInstance)")
	static FGameplayTagContainer& GetContextTagsFromCellLevelInstance(ANCellLevelInstance* LevelInstance)
	{
		return LevelInstance->GetContextTags();
	}
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Hex Seed (ANCellLevelInstance)")
	static FString GetHexSeedFromCellLevelInstance(ANCellLevelInstance* LevelInstance)
	{
		return FNSeedGenerator::HexFromSeed(LevelInstance->GetSeed());
	}
	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|WorldAssembly", DisplayName="Get Node Identifier (ANCellLevelInstance)")
	static int32 GetNodeIdentifierFromCellLevelInstance(ANCellLevelInstance* LevelInstance)
	{
		return LevelInstance->GetNodeIdentifier();
	}
};