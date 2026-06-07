// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NCoreMinimal.h"
#include "NEditorDefaults.h"
#include "Engine/DeveloperSettings.h"
#include "Macros/NEditorSettingsMacros.h"
#include "DelayedEditorTasks/NUpdateCheckDelayedEditorTask.h"
#include "NEditorSettings.generated.h"

/**
 * Project-level NEXUS editor settings (updates channel, update-check cadence).
 *
 * Saved to the NexusEditor config hierarchy and surfaced under Project Settings → NEXUS → Core.
 */
UCLASS(config = NexusEditor, defaultconfig)
class UNEditorSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	N_EDITOR_SETTINGS_BASE(UNEditorSettings, "Core", "General settings for the NEXUS: Core plugin.");

#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Check For Updates?", meta=(ToolTip="Should the NEXUS Framework check for updates periodically and notify you?"))
	bool bUpdatesCheck = true;
	
	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Frequency (Days)", meta=(ToolTip="How often, in days, to check for updates."))
	int32 UpdatesFrequency = 7;

	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Channel", meta=(ToolTip="Which release channel to check for updates against."))
	ENUpdatesChannel UpdatesChannel = ENUpdatesChannel::GithubRelease;

	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Custom Query URI", meta=(
		EditCondition="UpdatesChannel == ENUpdatesChannel::Custom", EditConditionHides,
		ToolTip="A fully qualified URI to the NCoreMinimal.h file in your custom fork, see NUpdatesMonitor::MainURI as an example."))
	FString UpdatesCustomQueryURI = TEXT("");

	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Custom Update URI", meta=(
		EditCondition="UpdatesChannel == ENUpdatesChannel::Custom", EditConditionHides,
		ToolTip="A fully qualified URI to open when an update is detected as avaialble."))
	FString UpdatesCustomUpdateURI = TEXT("");
	
	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Ignore Version Number <=", meta=(ToolTip="Updates with a version number at or below this value are ignored and will not trigger a notification."))
	int32 UpdatesIgnoreVersion = NEXUS::Version::Number;

#endif // WITH_EDITORONLY_DATA
};