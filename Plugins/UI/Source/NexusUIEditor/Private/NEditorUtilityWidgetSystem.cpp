#include "NEditorUtilityWidgetSystem.h"

#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "IBlutilityModule.h"
#include "LevelEditor.h"
#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetLoadTask.h"
#include "NUIEditorMinimal.h"

void UNEditorUtilityWidgetSystem::OnMapChanged(UWorld* World, EMapChangeType MapChange)
{
	switch (MapChange)
	{
		using enum EMapChangeType;
	case TearDownWorld:
		// Save ?
		break;
	case NewMap:
		// Restore
		break;
	case LoadMap:
		break;
	case SaveMap:
		break;
	}
}

void UNEditorUtilityWidgetSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UNEditorUtilityWidgetLoadTask::Create();
	
	// Bind to map change event
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
	MapChangeDelegateHandle = LevelEditorModule.OnMapChanged().AddUObject(this, &UNEditorUtilityWidgetSystem::OnMapChanged);
}

void UNEditorUtilityWidgetSystem::Deinitialize()
{
	// Unbind map change event
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.OnMapChanged().Remove(MapChangeDelegateHandle);
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
	
	UEditorUtilityWidgetBlueprint* EditorWidget = Cast<UEditorUtilityWidgetBlueprint>(TemplateDuplicate);
	if (EditorWidget != nullptr)
	{
		UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
		UEditorUtilityWidget* Widget = EditorUtilitySubsystem->SpawnAndRegisterTab(EditorWidget);
				
		// We dont want these tracked
		IBlutilityModule* BlutilityModule = FModuleManager::GetModulePtr<IBlutilityModule>("Blutility");
		BlutilityModule->RemoveLoadedScriptUI(EditorWidget);
		
		UNEditorUtilityWidget* UtilityWidget = Cast<UNEditorUtilityWidget>(Widget);
		if (UtilityWidget != nullptr)
		{
			UtilityWidget->PinTemplate(EditorWidget);
			RestoreWidgetState(UtilityWidget, Identifier, WidgetState);
			return UtilityWidget;
		}
	}
	return nullptr;
}

void UNEditorUtilityWidgetSystem::AddWidgetState(const FName& Identifier, const FString& Template, const FNWidgetState& WidgetState)
{
	// Check for already registered
	int32 WorkingIndex = GetIdentifierIndex(Identifier);
	if (WorkingIndex == INDEX_NONE)
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

void UNEditorUtilityWidgetSystem::RemoveWidgetState(const FName& Identifier)
{
	const int32 WorkingIndex = GetIdentifierIndex(Identifier);
	if (WorkingIndex != INDEX_NONE)
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

void UNEditorUtilityWidgetSystem::RestoreWidgetState(UNEditorUtilityWidget* Widget, const FName& Identifier,
	FNWidgetState& WidgetState)
{
	if (Widget->Implements<UNWidgetStateProvider>())
	{
		INWidgetStateProvider* StateProvider = Cast<INWidgetStateProvider>(Widget);
		if (StateProvider != nullptr)
		{
			StateProvider->RestoreWidgetState(Widget, Identifier, WidgetState);
		}
		else
		{
			INWidgetStateProvider::Execute_OnRestoreWidgetStateEvent(Widget, Identifier, WidgetState);
		}
	}
}

void UNEditorUtilityWidgetSystem::UpdateWidgetState(const FName& Identifier, const FNWidgetState& WidgetState)
{
	const int32 WorkingIndex = GetIdentifierIndex(Identifier);
	if (WorkingIndex != INDEX_NONE)
	{
		WidgetStates[WorkingIndex] = WidgetState;
		SaveConfig();
	}
	else
	{
		UE_LOG(LogNexusUIEditor, Error, TEXT("Failed to update widget(%s)' state; no registered widget was found."), *Identifier.ToString());
	}
}
