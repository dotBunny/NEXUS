// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonGameViewportClient.h"
#include "NGameViewportClient.generated.h"

class UGameInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnToggleWorldRenderingDelegate, bool, NewValue);

/**
 * A viewport with some base functionality added.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/types/game-viewport-client/">UNGameViewportClient</a>
 */
UCLASS(BlueprintType)
class NEXUSUI_API UNGameViewportClient : public UCommonGameViewportClient
{
	GENERATED_BODY()
	
	/**
	 * Toggles if the world should render.
	 * @remark This can be useful when you pause a game and want to show a fullscreen UI.
	 * @param bNewValue Should the world render? 
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Toggle World Rendering", Category = "NEXUS|User Interface",
		meta=(DocsURL="https://nexus-framework.com/docs/plugins/ui/types/game-viewport-client/#toggle-world-rendering"))
	void ToggleWorldRendering(const bool bNewValue)
	{
		OnDisableWorldRendering.Broadcast(bNewValue);
		bDisableWorldRendering = bNewValue;
	};

protected:

	UPROPERTY(BlueprintAssignable, Category = "NEXUS|User Interface")
	FOnToggleWorldRenderingDelegate OnDisableWorldRendering;
};
