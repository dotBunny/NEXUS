// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "EditorUtilityLibrary.h"
#include "NDelayedEditorTask.h"
#include "NDetailsRefreshDelayedEditorTask.generated.h"

UCLASS()
class NEXUSCOREEDITOR_API UNDetailsRefreshDelayedEditorTask : public UNDelayedEditorTask
{
	GENERATED_BODY()

public:
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