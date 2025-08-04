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
	UPROPERTY(EditAnywhere, config, Category = "Project", DisplayName = "AppIcon Path", meta=(ToolTip="Replaces the UnrealEditor AppIcon style, can be in SVG or image format. A restart of the editor is required for it to take effect."))
	FString AppIconPath;

	UPROPERTY(EditAnywhere, config, Category = "Project", DisplayName = "Window Icon Path", meta=(ToolTip="Replaces the UnrealEditor window icon at a platform level. This should simply be the path to the file WITHOUT any extension. The extension will be determined by the platform, thus all resources should have the same base name, and be located in the same folder. A restart of the editor is required for it to take effect."))
	FString WindowIconPath;
	
	UPROPERTY(EditAnywhere, config, Category = "Project", DisplayName = "Levels", meta = (AllowedClasses = "/Script/Engine.World"))
	TArray<FSoftObjectPath> ProjectLevels;
#endif
};