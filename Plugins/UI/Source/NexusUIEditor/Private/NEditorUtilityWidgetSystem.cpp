#include "NEditorUtilityWidgetSystem.h"

#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "IBlutilityModule.h"
#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetLoadTask.h"
#include "NUIEditorMinimal.h"

void UNEditorUtilityWidgetSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UNEditorUtilityWidgetLoadTask::Create();
}

UNEditorUtilityWidget* UNEditorUtilityWidgetSystem::CreateWithState(const FString& InTemplate, const FName& InIdentifier, FNWidgetState& WidgetState)
{
	const UBlueprint* TemplateWidget = LoadObject<UBlueprint>(nullptr, InTemplate);
	if (TemplateWidget == nullptr)
	{
		UE_LOG(LogNexusUIEditor, Warning, TEXT("Unable to create a UNEditorUtilityWidget as the provided UBlueprint(%s) was unable to load."), *InTemplate)
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
			if (UtilityWidget->Implements<UNWidgetStateProvider>())
			{
				if (INWidgetStateProvider* StateProvider = Cast<INWidgetStateProvider>(UtilityWidget); 
					StateProvider != nullptr)
				{
					StateProvider->RestoreWidgetState(UtilityWidget, Identifier, WidgetState);
				}
				else
				{
					INWidgetStateProvider::Execute_OnRestoreWidgetStateEvent(UtilityWidget, Identifier, WidgetState);
				}
			}
			return UtilityWidget;
		}
	}
	return nullptr;
}

void UNEditorUtilityWidgetSystem::RegisterWidget(const FName& Identifier, const FString& Template, const FNWidgetState& WidgetState)
{
	// Check for already registered
	if (int32 WorkingIndex = GetIdentifierIndex(Identifier); 
		WorkingIndex == INDEX_NONE)
	{
		// Add our items
		WorkingIndex = Identifiers.Add(Identifier);
		const int32 PayloadIndexCheck = WidgetStates.Add(WidgetState);
		const int32 TemplateIndexCheck = Templates.Add(Template);
		
		// Sanity check
		if (WorkingIndex != PayloadIndexCheck || WorkingIndex != TemplateIndexCheck)
		{
			UE_LOG(LogNexusUIEditor, Error, TEXT("Sanity check of the known registered widgets's arrays shows inconsistencies; data will be cleared."))
			Clear();
		}
	}
	else
	{
		UE_LOG(LogNexusUIEditor, Warning, TEXT("A widget is already registered for Identifier(%s); updating cached state only."), *Identifier.ToString());
		WidgetStates[WorkingIndex] = WidgetState;
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
		WidgetStates.RemoveAt(WorkingIndex);
		
		SaveConfig();
	}
	else
	{
		UE_LOG(LogNexusUIEditor, Warning, TEXT("Failed to unregister widget(%s); no registered widget was found."), *Identifier.ToString());
	}
}

void UNEditorUtilityWidgetSystem::UpdateWidgetState(const FName& Identifier, const FNWidgetState& WidgetState)
{
	if (const int32 WorkingIndex = GetIdentifierIndex(Identifier); 
		WorkingIndex != INDEX_NONE)
	{
		WidgetStates[WorkingIndex] = WidgetState;
		SaveConfig();
	}
	else
	{
		UE_LOG(LogNexusUIEditor, Error, TEXT("Failed to update widget(%s)' state; no registered widget was found."), *Identifier.ToString());
	}
}