#include "NEditorUtilityWidgetSystem.h"

#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "IBlutilityModule.h"

#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetLoadTask.h"
#include "NUIEditorMinimal.h"

TArray<UNEditorUtilityWidget*> UNEditorUtilityWidgetSystem::KnownWidgets;
bool UNEditorUtilityWidgetSystem::bIsOpeningMap = false;

void UNEditorUtilityWidgetSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// Bind to look for when we are changing worlds/maps and prepare to have to remake tabs
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (AssetEditorSubsystem != nullptr)
	{
		OnAssetEditorRequestedOpenHandle = AssetEditorSubsystem->OnAssetEditorRequestedOpen().AddUObject(this, &UNEditorUtilityWidgetSystem::OnAssetEditorRequestedOpen);	
	}
	FEditorDelegates::OnMapOpened.AddUObject(this, &UNEditorUtilityWidgetSystem::OnMapOpened);
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
	UE_LOG(LogNexusUIEditor, Warning, TEXT(" UNEditorUtilityWidgetSystem::OnAssetEditorRequestedOpen"));
	// We only want to do something when were opening worlds as an asset
	if (!Cast<UWorld>(Object)) return;
	
	bIsOpeningMap = true;
	
	// Record our transient rebuild data
	TransientStates.Clear();
	for (UNEditorUtilityWidget* Widget : KnownWidgets)
	{
		if (Widget->IsPersistent())
		{
			UE_LOG(LogNexusUIEditor, Warning, TEXT("RECORD STATE"));
			TransientStates.AddWidgetState(
				Widget->GetUserSettingsIdentifier(), 
				Widget->GetUserSettingsTemplate(), 
				Widget->GetState());
			
			UE_LOG(LogNexusUIEditor, Warning, TEXT("ID: %s"), *Widget->GetUserSettingsIdentifier().ToString());
			UE_LOG(LogNexusUIEditor, Warning, TEXT("Template: %s"), *Widget->GetUserSettingsTemplate());
		}
	}
	
	UE_LOG(LogNexusUIEditor, Warning, TEXT("Recording transient widget states for %d widgets"), TransientStates.Identifiers.Num());
}

void UNEditorUtilityWidgetSystem::OnMapOpened(const FString& Filename, bool bAsTemplate)
{
	UE_LOG(LogNexusUIEditor, Warning, TEXT("UNEditorUtilityWidgetSystem::OnMapOpened"));
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

void UNEditorUtilityWidgetSystem::RestoreTransientWidget(UNEditorUtilityWidget* Widget)
{
	if (TransientStates.GetCount() > 0)
	{
		int32 Index = TransientStates.GetCount() - 1;
		UE_LOG(LogNexusUIEditor, Log, TEXT("Restoring transient widget state for %s"), *TransientStates.Identifiers[Index].ToString());
		Widget->RestoreWidgetState(Widget, TransientStates.Identifiers[Index], TransientStates.WidgetStates[Index]);
		TransientStates.RemoveAtIndex(Index);
	}
}


void UNEditorUtilityWidgetSystem::RegisterWidget(UNEditorUtilityWidget* Widget)
{
	if (!KnownWidgets.Contains(Widget))
	{
		KnownWidgets.Add(Widget);
	}
}

void UNEditorUtilityWidgetSystem::UnregisterWidget(UNEditorUtilityWidget* Widget)
{
	if (KnownWidgets.Contains(Widget))
	{
		KnownWidgets.Remove(Widget);
	}
}

UNEditorUtilityWidget* UNEditorUtilityWidgetSystem::GetWidget(const FName& Identifier)
{
	for (const auto Widget : KnownWidgets)
	{
		if (Widget->GetUserSettingsIdentifier() == Identifier)
		{
			return Widget;
		}
	}
	return nullptr;
}

bool UNEditorUtilityWidgetSystem::HasWidget(const FName& Identifier)
{
	for (const auto Widget : KnownWidgets)
	{
		if (Widget->GetUserSettingsIdentifier() == Identifier)
		{
			return true;
		}
	}
	return false;
}
