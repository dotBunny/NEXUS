// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NEditorUtils.h"
#include "NToolsEditorUserSettings.generated.h"

UCLASS(config = NexusUserSettings, meta = (DisplayName = "Tools (User)"))
class NEXUSTOOLSEDITOR_API UNToolsEditorUserSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNToolsEditorUserSettings);

	static void Register() { FNEditorUtils::RegisterSettings(GetMutable()); }
	static void Unregister() { FNEditorUtils::UnregisterSettings(Get());}
	static void OnPostEngineInit();
	
	virtual FName GetContainerName() const override { return FNEditorDefaults::GetEditorSettingsContainerName(); }
	virtual FName GetCategoryName() const override {  return FNEditorDefaults::GetEditorSettingsCategoryName();  }
	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Tools (User)"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("Specific settings for NEXUS: Tools included with the framework."));
		return SectionDescription;
	}


#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, config, Category = "Leak Check", DisplayName ="Duration", meta=(ToolTip="How long should the leak check run?"))
	float LeakCheckTime = 30.f;
	
#endif // WITH_EDITORONLY_DATA

};