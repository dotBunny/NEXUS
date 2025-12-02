// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NEditorUtils.h"
#include "NEditorUtilityWidgetUserSettings.generated.h"

USTRUCT()
struct FNEditorUtilityWidgetSettings
{
	GENERATED_BODY()
	
	UPROPERTY()
	FString Identifier;
	
	UPROPERTY()
	FString Widget;
	
	UPROPERTY()
	FString JsonPayload;
};

UCLASS(config = EditorPerProjectUserSettings, meta = (DisplayName = "Editor Utility Widget"))
class NEXUSUIEDITOR_API UNEditorUtilityWidgetUserSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNEditorUtilityWidgetUserSettings);

	virtual bool SupportsAutoRegistration() const override
	{
		return false;
	}
	static void OnPostEngineInit();
	
	virtual FName GetContainerName() const override { return FNEditorDefaults::GetEditorSettingsContainerName(); }
	virtual FName GetCategoryName() const override {  return FNEditorDefaults::GetEditorSettingsCategoryName();  }
	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Editor Utility Widgets (User)"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("Cached data related to any NEditorUtilityWidgets."));
		return SectionDescription;
	}
	
	UPROPERTY()
	TArray<FNEditorUtilityWidgetSettings> Widgets;
};