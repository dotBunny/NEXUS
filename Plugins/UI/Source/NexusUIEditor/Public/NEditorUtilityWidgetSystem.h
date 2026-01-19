// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWidgetState.h"
#include "NWidgetStateSnapshot.h"
#include "Engine/DeveloperSettings.h"
#include "NEditorUtilityWidgetSystem.generated.h"

class UNEditorUtilityWidget;

// TODO Make Get for EditorSubsystem


/**
 * System to store state for any Widget that knows of the system.
 */
UCLASS(config = EditorPerProjectUserSettings)
class NEXUSUIEDITOR_API UNEditorUtilityWidgetSystem : public UEditorSubsystem
{
	friend class UNEditorUtilityWidgetLoadTask;

public:
	GENERATED_BODY()

	static bool IsRebuildingWidgets() { return bIsOpeningMap;}
	
	static void RegisterWidget(UNEditorUtilityWidget* Widget);
	static void UnregisterWidget(UNEditorUtilityWidget* Widget);
	static UNEditorUtilityWidget* GetWidget(const FName& Identifier);
	static bool HasWidget(const FName& Identifier);
	
	static UNEditorUtilityWidget* CreateWithState(const FString& InTemplate, const FName& InIdentifier, FNWidgetState& WidgetState);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void OnAssetEditorRequestedOpen(UObject* Object);
	void OnMapOpened(const FString& Filename, bool bAsTemplate);

	
	void AddWidgetState(const FName& Identifier, const FString& Template,  const FNWidgetState& WidgetState);
	void RemoveWidgetState(const FName& Identifier);
	void UpdateWidgetState(const FName& Identifier,  const FNWidgetState& WidgetState);
	bool HasWidgetState(const FName& Identifier) const;
	FNWidgetState& GetWidgetState(const FName& Identifier);
	
	void RestoreTransientWidget(UNEditorUtilityWidget* Widget);
	
protected:	
	UPROPERTY(config)   
	FNWidgetStateSnapshot WidgetStates;
	
	FNWidgetStateSnapshot TransientStates;
private:
	FDelegateHandle OnAssetEditorRequestedOpenHandle;
	FDelegateHandle OnMapOpenedHandle;
	
	static TArray<UNEditorUtilityWidget*> KnownWidgets;
	static bool bIsOpeningMap;
	
};

