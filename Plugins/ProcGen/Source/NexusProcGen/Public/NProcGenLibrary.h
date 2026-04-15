// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/NSeedGenerator.h"
#include "NProcGenLibrary.generated.h"

/**
 * @class UNProcGenLibrary
 * @brief A utility class providing functionality support ProcGen operations.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | ProcGen")
class UNProcGenLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Compares two arrays of actors to determine if they contain the same actors.
	 * 
	 * @param A The first array of actors to compare.
	 * @param B The second array of actors to compare.
	 * @return True if both arrays contain the same set of actors (regardless of order), false otherwise.
	 * 
	 * @note This function checks if both arrays have the same length and contain the same actor references,
	 *       regardless of their order in the arrays. Each actor in array A must only have exactly one matching
	 *       actor in array B, and vice versa.
	 */
	UFUNCTION(BlueprintPure, Category = "NEXUS|ProcGen", DisplayName="Get New Friendly Seed")
	static FString GetNewFriendlySeed() { return FNSeedGenerator::RandomFriendlySeed(); } 
};