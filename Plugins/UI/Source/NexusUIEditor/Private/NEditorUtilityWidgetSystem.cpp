#include "NEditorUtilityWidgetSystem.h"

#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "IBlutilityModule.h"

#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetLoadTask.h"
#include "NUIEditorMinimal.h"

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
	OnMapOpenedHandle = FEditorDelegates::OnMapOpened.AddStatic(&UNEditorUtilityWidgetSystem::OnMapOpened);
	
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
	
	// Unload all known blueprints
	WidgetBlueprints.Empty();
	
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
		AddPersistentState(Widget);
	}
}

void UNEditorUtilityWidgetSystem::OnMapOpened(const FString& Filename, bool bAsTemplate)
{
	bIsOpeningMap = false;
}

void UNEditorUtilityWidgetSystem::AddPersistentState(const TObjectPtr<UNEditorUtilityWidget> Widget)
{
	if (Widget->IsPersistent())
	{
		PersistentStates.AddWidgetState(
					Widget->GetUserSettingsIdentifier(), 
					Widget->GetUserSettingsTemplate(), 
					Widget->GetWidgetState(Widget));
		
		// Remove existing
		int32 HardState = WidgetStates.GetIdentifierIndex(Widget->GetUserSettingsIdentifier());
		if (HardState != INDEX_NONE)
		{
			WidgetStates.RemoveAtIndex(HardState);
			
		}
	}
}

UEditorUtilityWidget* UNEditorUtilityWidgetSystem::CreateWithState(const FString& InTemplate, const FName& InIdentifier, FNWidgetState& WidgetState)
{
	// Manage our spawned widgets were loading as they can be reused during different scenarios
	const TObjectPtr<UEditorUtilityWidgetBlueprint> NewWidget = Get()->GetWidgetBlueprint(InTemplate);
	
	FString IdentifierString = InIdentifier.ToString();
	IdentifierString.RemoveFromEnd(TEXT("_ActiveTab"));
	const FName Identifier(IdentifierString);
	
	UE_LOG(LogNexusUIEditor, Log, TEXT("Creating a new UNEditorUtilityWidget with identifier: %s"), *Identifier.ToString());
	
	// Spawn and make the tab for the widget
	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	UEditorUtilityWidget* Widget = EditorUtilitySubsystem->SpawnAndRegisterTabWithId(NewWidget.Get(), Identifier);
	
	// We don't want these tracked (Remove from EUW-list), opening it from there will just break the restoring
	IBlutilityModule* BlutilityModule = FModuleManager::GetModulePtr<IBlutilityModule>("Blutility");
	BlutilityModule->RemoveLoadedScriptUI(NewWidget.Get());
	
	// If it is one of our widgets, let's do a little extra
	UNEditorUtilityWidget* UtilityWidget = Cast<UNEditorUtilityWidget>(Widget);
	if (UtilityWidget != nullptr)
	{
		UtilityWidget->SetTemplate(NewWidget);
		
		// Attempt to automatically restore the widget state
		INWidgetStateProvider::InvokeRestoreWidgetState(UtilityWidget, Identifier, WidgetState);
		
		return UtilityWidget;
	}
	return Widget;
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

void UNEditorUtilityWidgetSystem::RestorePersistentWidget(const TObjectPtr<UNEditorUtilityWidget> Widget)
{
	const int32 TransientStateCount = PersistentStates.GetCount();
	if (TransientStateCount > 0)
	{
		const FString& WidgetTemplate = Widget->GetUserSettingsTemplate(); 
		for (int32 Index = TransientStateCount - 1; Index >= 0; Index--)
		{
			if (PersistentStates.Templates[Index] == WidgetTemplate)
			{
				const TObjectPtr<UEditorUtilityWidgetBlueprint> GetKnownBlueprint = GetWidgetBlueprint(PersistentStates.Templates[Index]);
				if (GetKnownBlueprint != nullptr)
				{
					Widget->SetTemplate(GetKnownBlueprint);
				}
				Widget->RestoreWidgetState(Widget, PersistentStates.Identifiers[Index], PersistentStates.WidgetStates[Index]);
				
				PersistentStates.RemoveAtIndex(Index);
				return;
			}
		}
	}
}

void UNEditorUtilityWidgetSystem::RegisterPersistentWidget(const TObjectPtr<UNEditorUtilityWidget> Widget)
{
	if (!PersistentWidgets.Contains(Widget))
	{
		PersistentWidgets.Add(Widget);
	}
}

void UNEditorUtilityWidgetSystem::UnregisterPersistentWidget(const TObjectPtr<UNEditorUtilityWidget> Widget)
{
	if (PersistentWidgets.Contains(Widget))
	{
		PersistentWidgets.Remove(Widget);
	}
}

TObjectPtr<UEditorUtilityWidgetBlueprint> UNEditorUtilityWidgetSystem::GetWidgetBlueprint(const FString& InTemplate)
{
	if (WidgetBlueprints.Contains(InTemplate))
	{
		return WidgetBlueprints[InTemplate];
	}

	const TObjectPtr<UEditorUtilityWidgetBlueprint> NewWidget = LoadObject<UEditorUtilityWidgetBlueprint>(this, InTemplate);
	if (NewWidget == nullptr)
	{
		UE_LOG(LogNexusUIEditor, Error, TEXT("Unable to create a UNEditorUtilityWidget as the provided UEditorUtilityWidgetBlueprint(%s) was unable to load."), *InTemplate)
		return nullptr;
	}
	return WidgetBlueprints.Add(InTemplate, NewWidget);
}