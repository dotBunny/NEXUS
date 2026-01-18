#include "NEditorUtilityWidgetSystem.h"

#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "IBlutilityModule.h"

#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetLoadTask.h"
#include "NUIEditorMinimal.h"

TMap<FName, INWidgetStateProvider*> UNEditorUtilityWidgetSystem::KnownWidgetStateProviders;

void UNEditorUtilityWidgetSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	MapChangedDelegateHandle = FEditorDelegates::MapChange.AddUObject(this, &UNEditorUtilityWidgetSystem::OnMapChanged);
	
	
	UNEditorUtilityWidgetLoadTask::Create();
}

void UNEditorUtilityWidgetSystem::Deinitialize()
{
	FEditorDelegates::MapChange.Remove(MapChangedDelegateHandle);
	Super::Deinitialize();
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

void UNEditorUtilityWidgetSystem::OnMapChanged(uint32 MapChangeFlags)
{
	// THIS HAPPENS AFTER THE WINDOW HAS ALREADY BEEN DESTROYED
	UE_LOG(LogNexusUIEditor, Log, TEXT("Map changed event fired. %i"), MapChangeFlags);
	SavePersistentWidgets();
	// if (MapChangeFlags == MapChangeEventFlags::WorldTornDown)
	// {
	// 	SavePersistentWidgets();
	// }
	// if ( MapChangeFlags == MapChangeEventFlags::NewMap)
	// {
	// 	RecreatePersistentWidgets();
	// }
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
			ClearStateData();
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




void UNEditorUtilityWidgetSystem::SavePersistentWidgets()
{
	PersistentIdentifiers.Empty();
	UE_LOG(LogNexusUIEditor, Warning, TEXT("Saving %d persistent widgets."), KnownWidgetStateProviders.Num());
	for (auto Provider : KnownWidgetStateProviders)
	{
		if (!HasWidgetState(Provider.Key))
		{
			UE_LOG(LogNexusUIEditor, Warning, TEXT("Unable to recreate widget(%s)."), *Provider.Key.ToString());
			continue;
		}

		UNEditorUtilityWidget* EUW = Cast<UNEditorUtilityWidget>(Provider.Value);
		if (EUW != nullptr)
		{
			UpdateWidgetState(Provider.Key, EUW->GetFullWidgetState());
		}
		else
		{
			UE_LOG(LogNexusUIEditor, Warning, TEXT("Unable to cast widget(%s)."), *Provider.Key.ToString());
		}
		
	}
}
void UNEditorUtilityWidgetSystem::RecreatePersistentWidgets()
{
	if (PersistentIdentifiers.Num() > 0)
	{
		for (auto Identifier : PersistentIdentifiers)
		{
			UE_LOG(LogNexusUIEditor, Log, TEXT("Creating persistent widget(%s)."), *Identifier.ToString());
	// 		UNWidgetEditorUtilityWidget* Widget = UNWidgetEditorUtilityWidget::GetEditorUtilityWidget(Identifier);
	// 		if (Widget != nullptr)
	// 		{
	// 			CreateWithState(Templates[GetIdentifierIndex(Identifier)], Identifier, WidgetStates[GetIdentifierIndex(Identifier)]);
	// 		}
		}
		PersistentIdentifiers.Empty();
	}
}

void UNEditorUtilityWidgetSystem::RegisterAsPersistent(const FName& Identifier,
	INWidgetStateProvider* WidgetStateProvider)
{
	KnownWidgetStateProviders.Add(Identifier, WidgetStateProvider);
}

void UNEditorUtilityWidgetSystem::UnregisterAsPersistent(const FName& Identifier,
	INWidgetStateProvider* WidgetStateProvider)
{
	KnownWidgetStateProviders.Remove(Identifier);
}
