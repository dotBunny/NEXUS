// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NEditorUtils.h"
#include "NEditorSettings.generated.h"

UCLASS(config = NexusEditor, defaultconfig)
class UNEditorSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_IMPLEMENT_EDITOR_SETTINGS(UNEditorSettings);
	
	static void Register() { FNEditorUtils::RegisterSettings(GetMutable());}
	static void Unregister() { FNEditorUtils::UnregisterSettings(Get());}
	
	virtual FName GetContainerName() const override { return FNEditorDefaults::GetEditorSettingsContainerName(); }
	virtual FName GetCategoryName() const override {  return FNEditorDefaults::GetEditorSettingsCategoryName();  }
	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Core"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("General framework settings for the Unreal Editor."));
		return SectionDescription;
	}

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, config, Category = "Quick Maps", DisplayName = "Core", meta = (AllowedClasses = "/Script/Engine.World"))
	TArray<FSoftObjectPath> CoreMaps;

	UPROPERTY(EditAnywhere, config, Category = "Quick Maps", DisplayName = "Gyms", meta = (AllowedClasses = "/Script/Engine.World"))
	TArray<FSoftObjectPath> GymMaps;
#endif
};