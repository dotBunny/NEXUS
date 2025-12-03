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

UNEditorUtilityWidget* UNEditorUtilityWidgetSystem::CreateWithState(const FString& InTemplate, const FName& InIdentifier, FNWidgetState& WidgetState)
{
	const UBlueprint* TemplateWidget = LoadObject<UBlueprint>(nullptr, InTemplate);
	if (TemplateWidget == nullptr)
	{
		NE_LOG_WARNING("[UNEditorUtilityWidgetSystem::CreateWithPayload] Unable to load template blueprint. (%s)", *InTemplate)
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
			NE_LOG_ERROR("[UNEditorUtilityWidgetSystem::RegisterWidget] Index count invalid, clearing cache.")
			Clear();
		}
	}
	else
	{
		NE_LOG_WARNING("[UNEditorUtilityWidgetSystem::RegisterWidget] Widget with identifier %s already registered, updating payload only.", *Identifier.ToString());
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
		NE_LOG_WARNING("[UNEditorUtilityWidgetSystem::UnregisterWidget] Unable to find widget with identifier %s", *Identifier.ToString());
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
		NE_LOG_ERROR("[UNEditorUtilityWidgetSystem::UpdatePayload] Unable to find widget with identifier %s", *Identifier.ToString());
	}
}