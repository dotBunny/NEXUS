// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCoreMinimal.h"
#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "NEditorUtils.h"
#include "DelayedEditorTasks/NUpdateCheckDelayedEditorTask.h"
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
	UPROPERTY(EditAnywhere, config, Category = "Editor Icon", DisplayName = "AppIcon Path", meta=(ToolTip="Replaces the Starship AppIcon style, can be in SVG or image format. A restart of the editor is required for it to take effect."))
	FString ProjectAppIconPath;

	UPROPERTY(EditAnywhere, config, Category = "Editor Icon", DisplayName = "Window Icon Path", meta=(ToolTip="Replaces the Unreal Editor window icon at a platform level. This should simply be the path to the file WITHOUT any extension. The extension will be determined by the platform, thus all resources should have the same base name, and be located in the same folder. A restart of the editor is required for it to take effect."))
	FString ProjectWindowIconPath;
	
	UPROPERTY(EditAnywhere, config, Category = "Project", DisplayName = "Levels", meta = (AllowedClasses = "/Script/Engine.World"))
	TArray<FSoftObjectPath> ProjectLevels;

	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Check For Updates?", meta=(ToolTip="Should the NEXUS Framework check for updates periodically and notify you?"))
	bool bUpdatesCheck = true;
	
	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Frequency (Days)")
	int32 UpdatesFrequency = 7;
	
	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Channel" )
	ENUpdatesChannel UpdatesChannel = ENUpdatesChannel::GithubRelease;

	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Custom Query URI", meta=(
		EditCondition="UpdatesChannel == ENUpdatesChannel::NUC_Custom", EditConditionHides,
		ToolTip="A fully qualified URI to the NCoreMinimal.h file in your custom fork, see NUpdatesMonitor::MainURI as an example."))
	FString UpdatesCustomQueryURI = TEXT("");

	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Custom Update URI", meta=(
		EditCondition="UpdatesChannel == ENUpdatesChannel::NUC_Custom", EditConditionHides,
		ToolTip="A fully qualified URI to open when an update is detected as avaialble."))
	FString UpdatesCustomUpdateURI = TEXT("");
	
	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Ignore Version Number <=")
	int32 UpdatesIgnoreVersion = NEXUS::Version::Number;
#endif
};