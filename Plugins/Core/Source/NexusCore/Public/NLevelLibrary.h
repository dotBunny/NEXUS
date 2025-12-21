// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NLevelLibrary.generated.h"

UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS: Level Library")
class UNLevelLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, DisplayName = "Get All Map Names", Category = "NEXUS|Level")
	static TArray<FString> GetAllMapNames(TArray<FString> SearchPaths);
};