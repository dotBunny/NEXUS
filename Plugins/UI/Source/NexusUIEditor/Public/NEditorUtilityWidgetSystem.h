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
	
	void AddWidgetState(const FName& Identifier, const FNWidgetState& WidgetState);
	void RemoveWidgetState(const FName& Identifier);
	void UpdateWidgetState(const FName& Identifier,  const FNWidgetState& WidgetState);
	bool HasWidgetState(const FName& Identifier) const;
	FNWidgetState& GetWidgetState(const FName& Identifier);

	void RegisterWidget(UNEditorUtilityWidget* Widget);
	void UnregisterWidget(const UNEditorUtilityWidget* Widget);
	UNEditorUtilityWidget* GetWidget(const FName& Identifier);
	bool HasWidget(const FName& Identifier);
protected:
	UPROPERTY(config)   
	FNWidgetStateSnapshot WidgetStates;
private:
	TMap<FName, TObjectPtr<UNEditorUtilityWidget>> KnownWidgets;
};

