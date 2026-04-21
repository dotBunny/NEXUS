// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NActorPoolsDeveloperOverlay.h"
#include "NEditorUtilityWidget.h"
#include "Components/Button.h"
#include "NActorPoolsEditorUtilityWidget.generated.h"

/**
 * The Actor Pools editor utility widget: hosts the developer overlay and exposes editor-only actions
 * (e.g. creating a new UNActorPoolSet) in a dockable editor tab.
 */
UCLASS(MinimalAPI, HideCategories = Object)
class UNActorPoolsEditorUtilityWidget : public UNEditorUtilityWidget
{
	GENERATED_BODY()


	virtual void NativeConstruct() override;

protected:

	/** Button handler that creates a new UNActorPoolSet asset in the current content browser path. */
	UFUNCTION()
	void OnCreateActorPoolSet();

	/** Bound developer overlay widget displaying live pool state. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNActorPoolsDeveloperOverlay> Overlay;

	/** "Create Actor Pool Set" button. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UButton> CreateAPSButton;
};