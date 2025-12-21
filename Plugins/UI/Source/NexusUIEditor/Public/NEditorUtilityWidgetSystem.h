// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NWidgetState.h"
#include "Engine/DeveloperSettings.h"
#include "NEditorUtilityWidgetSystem.generated.h"

USTRUCT(BlueprintType)
struct FNEditorUtilityWidgetPayload
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	TArray<FString> Strings;
};

class UNEditorUtilityWidget;

UCLASS(config = EditorPerProjectUserSettings)
class NEXUSUIEDITOR_API UNEditorUtilityWidgetSystem : public UEditorSubsystem
{
public:
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	static UNEditorUtilityWidget* CreateWithState(const FString& InTemplate, const FName& InIdentifier, FNWidgetState& WidgetState);
	
	void RegisterWidget(const FName& Identifier, const FString& Template,  const FNWidgetState& WidgetState);
	void UnregisterWidget(const FName& Identifier);
	void UpdateWidgetState(const FName& Identifier,  const FNWidgetState& WidgetState);
	
	void Clear()
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
};