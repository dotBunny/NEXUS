// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NEditorUtilityWidgetLoadTask.h"

#include "EditorUtilityLibrary.h"
#include "NCoreEditorMinimal.h"

#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetUserSettings.h"

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
	if (UNEditorUtilityWidgetUserSettings* Settings = UNEditorUtilityWidgetUserSettings::GetMutable())
	{
		const TArray<FName> IdentifiersCopy = Settings->Identifiers;
		const TArray<FString> TemplatesCopy = Settings->Templates;
		const TArray<FNEditorUtilityWidgetUserSettingsPayload> PayloadsCopy = Settings->Payloads;
		
		Settings->ClearCache();

		const int32 Count = IdentifiersCopy.Num();
		
		for (int i = 0; i < Count; i++)
		{
			const UBlueprint* TemplateWidget = LoadObject<UBlueprint>(nullptr, TemplatesCopy[i]);
			if (TemplateWidget == nullptr)
			{
				NE_LOG(Warning, TEXT("[UNEditorUtilityWidgetLoadTask::Execute] Unable to load template blueprint. (%s)"), *TemplatesCopy[i])
				continue;
			}
			
			FString IdentifierString = IdentifiersCopy[i].ToString();
			IdentifierString.RemoveFromEnd(TEXT("_ActiveTab"));
			const FName Identifier(IdentifierString);
			
			// Need to duplicate the base
			UBlueprint* TemplateDuplicate = DuplicateObject<UBlueprint>(TemplateWidget, TemplateWidget->GetOutermost(), Identifier);
			TemplateDuplicate->SetFlags(RF_Public | RF_Transient | RF_TextExportTransient | RF_DuplicateTransient);
	
			if (UEditorUtilityWidgetBlueprint* EditorWidget = Cast<UEditorUtilityWidgetBlueprint>(TemplateDuplicate))
			{
				UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
				UEditorUtilityWidget* Widget = EditorUtilitySubsystem->SpawnAndRegisterTab(EditorWidget);
				
				// We dont want these tracked
				EditorUtilitySubsystem->LoadedUIs.Remove(EditorWidget);
				
				if (UNEditorUtilityWidget* UtilityWidget = Cast<UNEditorUtilityWidget>(Widget); 
					UtilityWidget != nullptr)
				{
					UtilityWidget->PinTemplate(EditorWidget);
					UtilityWidget->RestoreFromUserSettingsPayload(Identifier, PayloadsCopy[i]);
				}
			}
		}
		
		Settings->SaveConfig();
	}
}