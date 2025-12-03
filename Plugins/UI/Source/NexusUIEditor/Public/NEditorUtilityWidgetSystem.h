// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

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

	static UNEditorUtilityWidget* CreateWithPayload(const FString& InTemplate, const FName& InIdentifier, const FNEditorUtilityWidgetPayload& Payload);
	
	void RegisterWidget(const FName& Identifier, const FString& Template, const FNEditorUtilityWidgetPayload& Payload);
	void UnregisterWidget(const FName& Identifier);
	void UpdatePayload(const FName& Identifier, const FNEditorUtilityWidgetPayload& Payload);
	
	void Clear()
	{
		Identifiers.Empty(); 
		Templates.Empty();
		Payloads.Empty();
	}
	
	UPROPERTY(config)
	TArray<FName> Identifiers;
	UPROPERTY(config)
	TArray<FString> Templates;	
	UPROPERTY(config)
	TArray<FNEditorUtilityWidgetPayload> Payloads;

private:
	int32 GetIdentifierIndex(const FName Identifier) const { return Identifiers.IndexOfByKey(Identifier);}
};