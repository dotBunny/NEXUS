// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "INWidgetStateProvider.h"
#include "NWidgetState.h"
#include "Engine/DeveloperSettings.h"
#include "NEditorUtilityWidgetSystem.generated.h"

class UNEditorUtilityWidget;


// use this to store a snap shot ? 
struct FNWidgetStateSnapshot
{
	TArray<FName> Identifiers;
	TArray<FString> Templates;	
	TArray<FNWidgetState> WidgetStates;
};


/**
 * System to store state for any Widget that knows of the system.
 */
UCLASS(config = EditorPerProjectUserSettings)
class NEXUSUIEDITOR_API UNEditorUtilityWidgetSystem : public UEditorSubsystem
{
public:
	GENERATED_BODY()

	
	static UNEditorUtilityWidget* CreateWithState(const FString& InTemplate, const FName& InIdentifier, FNWidgetState& WidgetState);
	
	void OnMapChanged(uint32 MapFlags);
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void AddWidgetState(const FName& Identifier, const FString& Template,  const FNWidgetState& WidgetState);
	void RemoveWidgetState(const FName& Identifier);
	void UpdateWidgetState(const FName& Identifier,  const FNWidgetState& WidgetState);
	bool HasWidgetState(const FName& Identifier) const
	{
		return Identifiers.Contains(Identifier);
	}
	
	static void RestoreWidgetState(UNEditorUtilityWidget* Widget, const FName& Identifier, FNWidgetState& WidgetState);
	
	void SavePersistentWidgets();
	void RecreatePersistentWidgets();
	
	static void RegisterAsPersistent(const FName& Identifier, INWidgetStateProvider* WidgetStateProvider);
	static void UnregisterAsPersistent(const FName& Identifier, INWidgetStateProvider* WidgetStateProvider);
	
	void ClearStateData()
	{
		Identifiers.Empty(); 
		Templates.Empty();
		WidgetStates.Empty();
	}
	
	
	
	UPROPERTY(config)
	TArray<FName> Identifiers;
	UPROPERTY(config)
	TArray<FString> Templates;	
	UPROPERTY(config)
	TArray<FNWidgetState> WidgetStates;

private:
	int32 GetIdentifierIndex(const FName Identifier) const { return Identifiers.IndexOfByKey(Identifier);}
	
	
	FDelegateHandle MapChangedDelegateHandle;
	TArray<FName> PersistentIdentifiers;
	
	
	static TMap<FName, INWidgetStateProvider*> KnownWidgetStateProviders;
};

