// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWidgetState.h"
#include "NWidgetStateSnapshot.h"
#include "NWidgetTabIdentifiers.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSubsystemMacros.h"
#include "NEditorUtilityWidgetSubsystem.generated.h"

class UEditorUtilityWidget;
class UEditorUtilityWidgetBlueprint;
class UNEditorUtilityWidget;

/**
 * System to store state for any Widget that knows of the system, and facilitate restoring that state in different scenarios.
 */
UCLASS(config = EditorPerProjectUserSettings)
class NEXUSUIEDITOR_API UNEditorUtilityWidgetSubsystem : public UEditorSubsystem
{
public:
	GENERATED_BODY()
	N_EDITOR_SUBSYSTEM(UNEditorUtilityWidgetSubsystem)
	
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

	UPROPERTY(config)
	FNWidgetTabIdentifiers WidgetTabIdentifiers;
private:
	TMap<FName, TObjectPtr<UNEditorUtilityWidget>> KnownWidgets;
};

