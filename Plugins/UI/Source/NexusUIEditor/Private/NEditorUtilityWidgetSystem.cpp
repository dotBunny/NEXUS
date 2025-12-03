#include "NEditorUtilityWidgetSystem.h"

#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "IBlutilityModule.h"
#include "NCoreEditorMinimal.h"
#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetLoadTask.h"

void UNEditorUtilityWidgetSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UNEditorUtilityWidgetLoadTask::Create();
}

UNEditorUtilityWidget* UNEditorUtilityWidgetSystem::CreateWithPayload(const FString& InTemplate, const FName& InIdentifier, const FNEditorUtilityWidgetPayload& Payload)
{
	const UBlueprint* TemplateWidget = LoadObject<UBlueprint>(nullptr, InTemplate);
	if (TemplateWidget == nullptr)
	{
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidgetSystem::CreateWithPayload] Unable to load template blueprint. (%s)"), *InTemplate)
		return nullptr;
	}
			
	FString IdentifierString = InIdentifier.ToString();
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
		IBlutilityModule* BlutilityModule = FModuleManager::GetModulePtr<IBlutilityModule>("Blutility");
		BlutilityModule->RemoveLoadedScriptUI(EditorWidget);
				
		if (UNEditorUtilityWidget* UtilityWidget = Cast<UNEditorUtilityWidget>(Widget); 
			UtilityWidget != nullptr)
		{
			UtilityWidget->PinTemplate(EditorWidget);
			UtilityWidget->RestoreFromUserSettingsPayload(Identifier, Payload);
			return UtilityWidget;
		}
	}
	return nullptr;
}

void UNEditorUtilityWidgetSystem::RegisterWidget(const FName& Identifier, const FString& Template, const FNEditorUtilityWidgetPayload& Payload)
{
	// Check for already registered
	if (int32 WorkingIndex = GetIdentifierIndex(Identifier); 
		WorkingIndex == INDEX_NONE)
	{
		// Add our items
		WorkingIndex = Identifiers.Add(Identifier);
		const int32 PayloadIndexCheck = Payloads.Add(Payload);
		const int32 TemplateIndexCheck = Templates.Add(Template);
		
		// Sanity check
		if (WorkingIndex != PayloadIndexCheck || WorkingIndex != TemplateIndexCheck)
		{
			NE_LOG(Error, TEXT("[UNEditorUtilityWidgetSystem::RegisterWidget] Index count invalid, clearing cache."))
			Clear();
		}
	}
	else
	{
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidgetSystem::RegisterWidget] Widget with identifier %s already registered, updating payload only."), *Identifier.ToString());
		Payloads[WorkingIndex] = Payload;
	}
	
	SaveConfig();

}

void UNEditorUtilityWidgetSystem::UnregisterWidget(const FName& Identifier)
{
	if (const int32 WorkingIndex = GetIdentifierIndex(Identifier); 
		WorkingIndex != INDEX_NONE)
	{
		Identifiers.RemoveAt(WorkingIndex);
		Templates.RemoveAt(WorkingIndex);
		Payloads.RemoveAt(WorkingIndex);
		
		SaveConfig();
	}
	else
	{
		NE_LOG(Warning, TEXT("[UNEditorUtilityWidgetSystem::UnregisterWidget] Unable to find widget with identifier %s"), *Identifier.ToString());
	}
}

void UNEditorUtilityWidgetSystem::UpdatePayload(const FName& Identifier, const FNEditorUtilityWidgetPayload& Payload)
{
	if (const int32 WorkingIndex = GetIdentifierIndex(Identifier); 
		WorkingIndex != INDEX_NONE)
	{
		Payloads[WorkingIndex] = Payload;
		SaveConfig();
	}
	else
	{
		NE_LOG(Error, TEXT("[UNEditorUtilityWidgetSystem::UpdatePayload] Unable to find widget with identifier %s"), *Identifier.ToString());
	}
}