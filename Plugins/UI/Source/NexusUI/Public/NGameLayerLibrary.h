// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Visibility.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "NGameLayerLibrary.generated.h"

/**
 * A collection of functionality used to interact with a games' UGameUserSettings helping build UI components around them.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/types/game-user-settings-library/">UNGameUserSettingsLibrary</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "Game User Settings Library")
class UNGameLayerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	// TODO: Need to make localplayer required
	
public:	
	UFUNCTION(BlueprintCallable, Category = "NEXUS|UI|Game Layer")
	static bool SetLayerVisibility(UPARAM(ref) ULocalPlayer* LocalPlayer, const FName Name, ESlateVisibility Visibility);
};