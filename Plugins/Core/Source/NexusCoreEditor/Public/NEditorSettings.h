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

	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Check For Updates?", meta=(ToolTip="Should the NEXUS Framework check for updates periodically and notify you?"))
	bool bUpdatesCheck = true;
	
	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Frequency (Days)")
	int32 UpdatesFrequency = 7;
	
	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Channel" )
	ENUpdatesChannel UpdatesChannel = ENUpdatesChannel::GithubRelease;

	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Custom Query URI", meta=(
		EditCondition="UpdatesChannel == ENUpdatesChannel::Custom", EditConditionHides,
		ToolTip="A fully qualified URI to the NCoreMinimal.h file in your custom fork, see NUpdatesMonitor::MainURI as an example."))
	FString UpdatesCustomQueryURI = TEXT("");

	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Custom Update URI", meta=(
		EditCondition="UpdatesChannel == ENUpdatesChannel::Custom", EditConditionHides,
		ToolTip="A fully qualified URI to open when an update is detected as avaialble."))
	FString UpdatesCustomUpdateURI = TEXT("");
	
	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Ignore Version Number <=")
	int32 UpdatesIgnoreVersion = NEXUS::Version::Number;
#endif
};