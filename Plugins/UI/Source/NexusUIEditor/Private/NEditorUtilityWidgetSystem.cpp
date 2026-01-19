#include "NEditorUtilityWidgetSystem.h"

#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "IBlutilityModule.h"

#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetLoadTask.h"
#include "NUIEditorMinimal.h"

TArray<UNEditorUtilityWidget*> UNEditorUtilityWidgetSystem::PersistentWidgets;
FNWidgetStateSnapshot UNEditorUtilityWidgetSystem::PersistentStates;
bool UNEditorUtilityWidgetSystem::bIsOpeningMap = false;

void UNEditorUtilityWidgetSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// Bind to look for when we are changing worlds/maps and prepare to have to remake tabs
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (AssetEditorSubsystem != nullptr)
	{
		OnAssetEditorRequestedOpenHandle = AssetEditorSubsystem->OnAssetEditorRequestedOpen().AddStatic(&UNEditorUtilityWidgetSystem::OnAssetEditorRequestedOpen);	
	}
	FEditorDelegates::OnMapOpened.AddStatic(&UNEditorUtilityWidgetSystem::OnMapOpened);
	UNEditorUtilityWidgetLoadTask::Create();
}

void UNEditorUtilityWidgetSystem::Deinitialize()
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (AssetEditorSubsystem != nullptr)
	{
		AssetEditorSubsystem->OnAssetEditorRequestedOpen().Remove(OnAssetEditorRequestedOpenHandle);
	}
	FEditorDelegates::OnMapOpened.Remove(OnMapOpenedHandle);
	
	Super::Deinitialize();
}

void UNEditorUtilityWidgetSystem::OnAssetEditorRequestedOpen(UObject* Object)
{
	// We only want to do something when were opening worlds as an asset
	if (!Cast<UWorld>(Object)) return;
	
	bIsOpeningMap = true;
	
	// Record our transient rebuild data
	PersistentStates.Clear();
	for (UNEditorUtilityWidget* Widget : PersistentWidgets)
	{
		if (Widget->IsPersistent())
		{
			PersistentStates.AddWidgetState(
				Widget->GetUserSettingsIdentifier(), 
				Widget->GetUserSettingsTemplate(), 
				Widget->GetWidgetState(Widget));
		}
	}
}

void UNEditorUtilityWidgetSystem::OnMapOpened(const FString& Filename, bool bAsTemplate)
{
	bIsOpeningMap = false;
}

UNEditorUtilityWidget* UNEditorUtilityWidgetSystem::CreateWithState(const FString& InTemplate, const FName& InIdentifier, FNWidgetState& WidgetState)
{
	// Load as 
	
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
	UBlueprint* TemplateDuplicate = DuplicateObject<UBlueprint>(TemplateWidget, TemplateWidget->GetOutermost(), NAME_None);
	TemplateDuplicate->SetFlags(RF_Public | RF_Transient | RF_TextExportTransient | RF_DuplicateTransient);
	
	UEditorUtilityWidgetBlueprint* EditorWidget = Cast<UEditorUtilityWidgetBlueprint>(TemplateDuplicate);
	if (EditorWidget != nullptr)
	{
		UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
		UEditorUtilityWidget* Widget = EditorUtilitySubsystem->SpawnAndRegisterTab(EditorWidget);
				
		// We dont want these tracked (Remove from EUW window list), opening it from there will just break the restoring
		IBlutilityModule* BlutilityModule = FModuleManager::GetModulePtr<IBlutilityModule>("Blutility");
		BlutilityModule->RemoveLoadedScriptUI(EditorWidget);
		
		UNEditorUtilityWidget* UtilityWidget = Cast<UNEditorUtilityWidget>(Widget);
		if (UtilityWidget != nullptr)
		{
			
			UtilityWidget->PinTemplate(EditorWidget);
			
			// Attempt to automatically restore the widget state
			if (UtilityWidget->Implements<UNWidgetStateProvider>())
			{
				INWidgetStateProvider* StateProvider = Cast<INWidgetStateProvider>(UtilityWidget);
				if (StateProvider != nullptr)
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


void UNEditorUtilityWidgetSystem::AddWidgetState(const FName& Identifier, const FString& Template, const FNWidgetState& WidgetState)
{
	if (WidgetStates.AddWidgetState(Identifier, Template, WidgetState))
	{
		SaveConfig();
	}
}

void UNEditorUtilityWidgetSystem::RemoveWidgetState(const FName& Identifier)
{
	if (WidgetStates.RemoveWidgetState(Identifier))
	{
		SaveConfig();
	}
}

void UNEditorUtilityWidgetSystem::UpdateWidgetState(const FName& Identifier, const FNWidgetState& WidgetState)
{
	if (WidgetStates.UpdateWidgetState(Identifier, WidgetState))
	{
		SaveConfig();
	}
}

bool UNEditorUtilityWidgetSystem::HasWidgetState(const FName& Identifier) const
{
	return WidgetStates.HasWidgetState(Identifier);
}

FNWidgetState& UNEditorUtilityWidgetSystem::GetWidgetState(const FName& Identifier)
{
	return WidgetStates.GetWidgetState(Identifier);
}

void UNEditorUtilityWidgetSystem::RestorePersistentWidget(UNEditorUtilityWidget* Widget)
{
	int32 TransientStateCount = PersistentStates.GetCount();
	if (TransientStateCount > 0)
	{
		const FString& WidgetTemplate = Widget->GetUserSettingsTemplate(); 
		for (int32 Index = TransientStateCount - 1; Index >= 0; Index--)
		{
			if (PersistentStates.Templates[Index] == WidgetTemplate)
			{
				Widget->RestoreWidgetState(Widget, PersistentStates.Identifiers[Index], PersistentStates.WidgetStates[Index]);
				PersistentStates.RemoveAtIndex(Index);
			}
		}
	}
}


void UNEditorUtilityWidgetSystem::RegisterPersistentWidget(UNEditorUtilityWidget* Widget)
{
	if (!PersistentWidgets.Contains(Widget))
	{
		PersistentWidgets.Add(Widget);
	}
}

void UNEditorUtilityWidgetSystem::UnregisterPersistentWidget(UNEditorUtilityWidget* Widget)
{
	if (PersistentWidgets.Contains(Widget))
	{
		PersistentWidgets.Remove(Widget);
	}
}