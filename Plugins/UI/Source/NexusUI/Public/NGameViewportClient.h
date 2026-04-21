// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonGameViewportClient.h"
#include "NGameViewportClient.generated.h"

class UGameInstance;

/** Broadcast whenever the world-rendering toggle is flipped on or off. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnToggleWorldRenderingDelegate, bool, NewValue);

/**
 * A viewport with some base functionality added.
 * @see <a href="https://nexus-framework.com/docs/plugins/ui/types/game-viewport-client/">UNGameViewportClient</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Game Viewport Client", BlueprintType)
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

	/** Fires when ToggleWorldRendering is called, before bDisableWorldRendering is updated. */
	UPROPERTY(BlueprintAssignable, Category = "NEXUS|User Interface")
	FOnToggleWorldRenderingDelegate OnDisableWorldRendering;
};
