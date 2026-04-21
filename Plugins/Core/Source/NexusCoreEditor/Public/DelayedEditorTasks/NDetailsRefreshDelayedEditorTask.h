// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorUtilityLibrary.h"
#include "NDelayedEditorTask.h"
#include "NDetailsRefreshDelayedEditorTask.generated.h"

/**
 * Delayed editor task that forces the Details panel to reload its property customizations.
 *
 * Useful after registering or unregistering a detail customization at runtime — the panel
 * needs a refresh tick to pick up the change.
 */
UCLASS()
class NEXUSCOREEDITOR_API UNDetailsRefreshDelayedEditorTask : public UNDelayedEditorTask
{
	GENERATED_BODY()

public:
	/** Schedules the refresh to run after a short delay (0.5s, 5 ticks). */
	UFUNCTION(BlueprintCallable)
	static void Create()
	{
		UAsyncEditorDelay* DelayedMechanism = CreateDelayMechanism();
		UNDetailsRefreshDelayedEditorTask* PropertyObject = NewObject<UNDetailsRefreshDelayedEditorTask>(DelayedMechanism);
		PropertyObject->Lock(DelayedMechanism);
		
		DelayedMechanism->Complete.AddDynamic(PropertyObject, &UNDetailsRefreshDelayedEditorTask::Execute);
		DelayedMechanism->Start(0.5f, 5);
	}
	
private:	
	UFUNCTION()
	void Execute()
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.NotifyCustomizationModuleChanged();

		Release();
	}
};