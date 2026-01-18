#include "NEditorUtilityWidgetSystem.h"

#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "IBlutilityModule.h"

#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetLoadTask.h"
#include "NUIEditorMinimal.h"

TArray<UNEditorUtilityWidget*> UNEditorUtilityWidgetSystem::KnownWidgets;

void UNEditorUtilityWidgetSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	OnMapOpenedHandle = FEditorDelegates::OnMapOpened.AddUObject(this, &UNEditorUtilityWidgetSystem::OnMapOpened);
	
	// Bind to look for when we are changing worlds/maps and prepare to have to remake tabs
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (AssetEditorSubsystem != nullptr)
	{
		OnAssetEditorRequestedOpenHandle = AssetEditorSubsystem->OnAssetEditorRequestedOpen().AddUObject(this, &UNEditorUtilityWidgetSystem::OnAssetEditorRequestedOpen);	
	}

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
	UBlueprint* TemplateDuplicate = DuplicateObject<UBlueprint>(TemplateWidget, TemplateWidget->GetOutermost(), NAME_None);
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
	if (SavedState.AddWidgetState(Identifier, Template, WidgetState))
	{
		SaveConfig();
	}
}

void UNEditorUtilityWidgetSystem::RemoveWidgetState(const FName& Identifier)
{
	if (SavedState.RemoveWidgetState(Identifier))
	{
		SaveConfig();
	}
}

void UNEditorUtilityWidgetSystem::UpdateWidgetState(const FName& Identifier, const FNWidgetState& WidgetState)
{
	if (SavedState.UpdateWidgetState(Identifier, WidgetState))
	{
		SaveConfig();
	}
}

bool UNEditorUtilityWidgetSystem::HasWidgetState(const FName& Identifier) const
{
	return SavedState.HasWidgetState(Identifier);
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

void UNEditorUtilityWidgetSystem::OnAssetEditorRequestedOpen(UObject* Object)
{
	if (Cast<UWorld>(Object) == nullptr) return;
	
	// Were about to lose our world, we need to save things
	TemporaryState.Clear();
	for (const auto Widget : KnownWidgets)
	{
		if (!Widget->IsPersistent())
		{
			continue;
		};
		
		// Add to our temp cache
		TemporaryState.AddWidgetState(
			Widget->GetUserSettingsIdentifier(), 
			Widget->GetUserSettingsTemplate(),
			Widget->GetState());
	}
}

void UNEditorUtilityWidgetSystem::OnMapOpened(const FString& Filename, bool bAsTemplate)
{
	// Don't recreate 
	if (bAsTemplate) return;
	
	// Restore things
	const int Count = TemporaryState.GetCount();
	for (int i = 0; i < Count; ++i)
	{
		
		CreateWithState(TemporaryState.Templates[i], TemporaryState.Identifiers[i], TemporaryState.WidgetStates[i]);
	}
	TemporaryState.Clear();
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
