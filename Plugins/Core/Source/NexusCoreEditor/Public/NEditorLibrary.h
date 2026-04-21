// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NEditorUtils.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NEditorLibrary.generated.h"

/**
 * Blueprint-exposed editor helpers backed by FNEditorUtils.
 * @note Intended for editor-only utility blueprints and tooling; these calls are no-ops at runtime.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Editor Library")
class UNEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Selects Actor in the active editor viewport, replacing the current selection.
	 * @param Actor The actor to select.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Select Actor", Category = "NEXUS|Editor")
	static void SelectActor(AActor* Actor)
	{
		FNEditorUtils::SelectActor(Actor);
	}
};