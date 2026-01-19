// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorUtilityWidgetLoadTask.h"

#include "EditorUtilityLibrary.h"
#include "NEditorUtilityWidgetSystem.h"

void UNEditorUtilityWidgetLoadTask::Create()
{
	UAsyncEditorDelay* DelayedMechanism = CreateDelayMechanism();
	UNEditorUtilityWidgetLoadTask* UpdateObject = NewObject<UNEditorUtilityWidgetLoadTask>(DelayedMechanism);
	UpdateObject->Lock(DelayedMechanism);
	
	DelayedMechanism->Complete.AddDynamic(UpdateObject, &UNEditorUtilityWidgetLoadTask::Execute);
	DelayedMechanism->Start(0.25f, 1);
}

void UNEditorUtilityWidgetLoadTask::Execute()
{
	UNEditorUtilityWidgetSystem* System = UNEditorUtilityWidgetSystem::Get();
	if (System != nullptr)
	{
		const TArray<FName> IdentifiersCopy = System->WidgetStates.Identifiers;
		const TArray<FString> TemplatesCopy = System->WidgetStates.Templates;
		TArray<FNWidgetState> WidgetStatesCopy = System->WidgetStates.WidgetStates;
		
		System->WidgetStates.Clear();
		
		const int32 Count = IdentifiersCopy.Num();
		
		for (int i = 0; i < Count; i++)
		{
			UNEditorUtilityWidgetSystem::CreateWithState(TemplatesCopy[i], IdentifiersCopy[i], WidgetStatesCopy[i]);
		}
		
		System->SaveConfig();
	}
}