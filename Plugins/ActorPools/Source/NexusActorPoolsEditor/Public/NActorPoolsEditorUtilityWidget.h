// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NActorPoolsDeveloperOverlay.h"
#include "NEditorUtilityWidget.h"
#include "Components/Button.h"
#include "NActorPoolsEditorUtilityWidget.generated.h"

UCLASS(MinimalAPI, HideCategories = Object)
class UNActorPoolsEditorUtilityWidget : public UNEditorUtilityWidget
{
	GENERATED_BODY()

	
	virtual void NativeConstruct() override;

protected:	
	
	UFUNCTION()
	void OnCreateActorPoolSet();
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNActorPoolsDeveloperOverlay> Overlay;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UButton> CreateAPSButton;
};