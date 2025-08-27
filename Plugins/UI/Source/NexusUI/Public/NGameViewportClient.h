// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonGameViewportClient.h"
#include "NGameViewportClient.generated.h"

class UGameInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnToggleWorldRenderingDelegate, bool, NewValue);

UCLASS(BlueprintType)
class NEXUSUI_API UNGameViewportClient : public UCommonGameViewportClient
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "NEXUS|User Interface")
	void ToggleWorldRendering(const bool bNewValue)
	{
		OnDisableWorldRendering.Broadcast(bNewValue);
		bDisableWorldRendering = bNewValue;
	};

protected:

	UPROPERTY(BlueprintAssignable, Category = "NEXUS|User Interface")
	FOnToggleWorldRenderingDelegate OnDisableWorldRendering;
};
