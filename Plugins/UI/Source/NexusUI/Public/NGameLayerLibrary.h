// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Visibility.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "NGameLayerLibrary.generated.h"

/**
 * A collection of functionality used to interact with a games' IGameLayerManager.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/types/game-layer-library/">UNGameUserSettingsLibrary</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "Game Layer Library")
class UNGameLayerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/**
	 * Changes a layer's visibility from the GameLayerManager.
	 * @param LocalPlayer The local player whose UI you want to affect.
	 * @param Name The name of the layer you want to affect.
	 * @param Visibility The visibility you want to set the layer to.
	 * @return True if the layer was successfully set, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|UI|Game Layer")
	static bool SetLayerVisibility(UPARAM(ref) ULocalPlayer* LocalPlayer, const FName Name, ESlateVisibility Visibility);
};