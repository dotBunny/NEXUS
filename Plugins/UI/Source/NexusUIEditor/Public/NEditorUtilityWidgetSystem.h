// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWidgetState.h"
#include "NWidgetStateSnapshot.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSubsystemMacros.h"
#include "NEditorUtilityWidgetSystem.generated.h"

class UEditorUtilityWidget;
class UEditorUtilityWidgetBlueprint;
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

	static UEditorUtilityWidget* CreateWithState(const FString& InTemplate, const FName& InIdentifier, FNWidgetState& WidgetState);
	static bool IsRebuildingWidgets() { return bIsOpeningMap;}
	static void OnMapOpened(const FString& Filename, bool bAsTemplate);
	
	void OnAssetEditorRequestedOpen(UObject* Object);
	
	void RegisterPersistentWidget(TObjectPtr<UNEditorUtilityWidget> Widget);
	void UnregisterPersistentWidget(TObjectPtr<UNEditorUtilityWidget> Widget);
	void AddPersistentState(TObjectPtr<UNEditorUtilityWidget> Widget);
	void RestorePersistentWidget(TObjectPtr<UNEditorUtilityWidget> Widget);
	
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
	
	UPROPERTY()
	TMap<FString, TObjectPtr<UEditorUtilityWidgetBlueprint>> WidgetBlueprints;
	
	UPROPERTY()
	TArray<TObjectPtr<UNEditorUtilityWidget>> PersistentWidgets;
	
	UPROPERTY()
	FNWidgetStateSnapshot PersistentStates;
	
private:
	TObjectPtr<UEditorUtilityWidgetBlueprint> GetWidgetBlueprint(const FString& InTemplate);
	
	static bool bIsOpeningMap;
	
	FDelegateHandle OnAssetEditorRequestedOpenHandle;
	FDelegateHandle OnMapOpenedHandle;
};

