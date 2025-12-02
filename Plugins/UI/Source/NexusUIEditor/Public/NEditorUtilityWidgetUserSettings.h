// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NEditorUtilityWidgetUserSettings.generated.h"

USTRUCT(BlueprintType)
struct FNEditorUtilityWidgetUserSettingsPayload
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	TArray<FString> Strings;
};

UCLASS(config = EditorPerProjectUserSettings, meta = (DisplayName = "EUW User Settings"))
class NEXUSUIEDITOR_API UNEditorUtilityWidgetUserSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNEditorUtilityWidgetUserSettings);

	virtual FName GetContainerName() const override { return FNEditorDefaults::GetEditorSettingsContainerName(); }
	virtual FName GetCategoryName() const override {  return FNEditorDefaults::GetEditorSettingsCategoryName();  }
	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("EUW Settings (User)"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("Cached data related to any NEditorUtilityWidgets."));
		return SectionDescription;
	}
	
	void RegisterWidget(const FName& Identifier, const FString& Template, const FNEditorUtilityWidgetUserSettingsPayload& Payload);
	void UnregisterWidget(const FName& Identifier);
	void UpdatePayload(const FName& Identifier, const FNEditorUtilityWidgetUserSettingsPayload& Payload);
	
	void ClearCache()
	{
		Identifiers.Empty(); 
		Templates.Empty();
		Payloads.Empty();
	}
	
	UPROPERTY(VisibleAnywhere, config, Category = "Cache")
	TArray<FName> Identifiers;
	UPROPERTY(VisibleAnywhere, config, Category = "Cache")
	TArray<FString> Templates;	
	UPROPERTY(VisibleAnywhere, config, Category = "Cache")
	TArray<FNEditorUtilityWidgetUserSettingsPayload> Payloads;

private:
	int32 GetIdentifierIndex(const FName Identifier) const { return Identifiers.IndexOfByKey(Identifier);}
};