// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NActorLibrary.generated.h"

/**
 * @class UNActorLibrary
 * @brief A utility class providing core functions and operations for Actors.
 *
 * The UNActorLibrary class serves as a collection of static utility functions
 * for working with Actors in Unreal Engine. These functions are designed to be 
 * callable from both C++ and Blueprint contexts, providing common operations
 * related to Actor manipulation, comparison, and management.
 */
UCLASS()
class UNActorLibrary : public UBlueprintFunctionLibrary
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
	UFUNCTION(BlueprintCallable, Category = "NEXUS|Actor")
	static bool IsSameActors(const TArray<AActor*>& A, const TArray<AActor*>& B);
};