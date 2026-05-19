// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
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
};