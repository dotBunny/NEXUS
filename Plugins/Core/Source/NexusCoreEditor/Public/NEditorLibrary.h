// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NEditorUtils.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NEditorLibrary.generated.h"

UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS: Editor Library")
class UNEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, DisplayName = "Select Actor", Category = "NEXUS|Editor")
	static void SelectActor(AActor* Actor)
	{
		FNEditorUtils::SelectActor(Actor);
	}
};