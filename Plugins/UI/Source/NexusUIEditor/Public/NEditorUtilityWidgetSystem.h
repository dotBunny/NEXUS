// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWidgetState.h"
#include "NWidgetStateSnapshot.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSubsystemMacros.h"
#include "NEditorUtilityWidgetSystem.generated.h"

class UNEditorUtilityWidget;

/**
 * System to store state for any Widget that knows of the system, and facilitate restoring that state in different scenarios.
 */
UCLASS(config = EditorPerProjectUserSettings)
class NEXUSUIEDITOR_API UNEditorUtilityWidgetSystem : public UEditorSubsystem
{
	friend class UNEditorUtilityWidgetLoadTask;

public:
	GENERATED_BODY()
	N_EDITOR_SUBSYSTEM(UNEditorUtilityWidgetSystem)

	static bool IsRebuildingWidgets() { return bIsOpeningMap;}
	static void RegisterPersistentWidget(UNEditorUtilityWidget* Widget);
	static void UnregisterPersistentWidget(UNEditorUtilityWidget* Widget);
	static void RestorePersistentWidget(UNEditorUtilityWidget* Widget);
	
	static void OnAssetEditorRequestedOpen(UObject* Object);
	static void OnMapOpened(const FString& Filename, bool bAsTemplate);
	
	static UNEditorUtilityWidget* CreateWithState(const FString& InTemplate, const FName& InIdentifier, FNWidgetState& WidgetState);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void AddWidgetState(const FName& Identifier, const FString& Template,  const FNWidgetState& WidgetState);
	void RemoveWidgetState(const FName& Identifier);
	void UpdateWidgetState(const FName& Identifier,  const FNWidgetState& WidgetState);
	bool HasWidgetState(const FName& Identifier) const;
	FNWidgetState& GetWidgetState(const FName& Identifier);

protected:	
	UPROPERTY(config)   
	FNWidgetStateSnapshot WidgetStates;
	
private:
	static TArray<UNEditorUtilityWidget*> PersistentWidgets;
	static FNWidgetStateSnapshot PersistentStates;
	static bool bIsOpeningMap;
	
	FDelegateHandle OnAssetEditorRequestedOpenHandle;
	FDelegateHandle OnMapOpenedHandle;
};

