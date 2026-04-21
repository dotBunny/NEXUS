// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NLevelLibrary.generated.h"

/**
 * A utility class providing Blueprint-callable level/map operations.
 *
 * The UNLevelLibrary class exposes level-related helpers (such as discovering maps on disk)
 * to Blueprint scripts. For C++ callers, prefer the equivalents in FNLevelUtils.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Level Library")
class UNLevelLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Enumerates all known map package names reachable from the supplied search paths.
	 * @param SearchPaths A list of content-root relative paths (e.g. "/Game/Maps") to scan for maps.
	 * @return An array of package names of all maps found underneath the search paths.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Get All Map Names", Category = "NEXUS|Level")
	static TArray<FString> GetAllMapNames(TArray<FString> SearchPaths);
};