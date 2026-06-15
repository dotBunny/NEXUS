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

	/** Master switch: when true the framework periodically checks for updates and notifies the user. */
	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Check For Updates?", meta=(ToolTip="Should the NEXUS Framework check for updates periodically and notify you?"))
	bool bUpdatesCheck = true;

	/** How often, in days, the update check runs. */
	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Frequency (Days)", meta=(ToolTip="How often, in days, to check for updates."))
	int32 UpdatesFrequency = 7;

	/** Release channel the update check queries. */
	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Channel", meta=(ToolTip="Which release channel to check for updates against."))
	ENUpdatesChannel UpdatesChannel = ENUpdatesChannel::GithubRelease;

	/** Custom channel only: fully-qualified URI of the NCoreMinimal.h to read the version from. */
	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Custom Query URI", meta=(
		EditCondition="UpdatesChannel == ENUpdatesChannel::Custom", EditConditionHides,
		ToolTip="A fully qualified URI to the NCoreMinimal.h file in your custom fork, see NUpdatesMonitor::MainURI as an example."))
	FString UpdatesCustomQueryURI = TEXT("");

	/** Custom channel only: fully-qualified URI opened when an update is found. */
	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Custom Update URI", meta=(
		EditCondition="UpdatesChannel == ENUpdatesChannel::Custom", EditConditionHides,
		ToolTip="A fully qualified URI to open when an update is detected as avaialble."))
	FString UpdatesCustomUpdateURI = TEXT("");

	/** Suppression threshold: releases with a version number at or below this value never trigger a notification. */
	UPROPERTY(EditAnywhere, Config, Category = "Updates", DisplayName = "Ignore Version Number <=", meta=(ToolTip="Updates with a version number at or below this value are ignored and will not trigger a notification."))
	int32 UpdatesIgnoreVersion = NEXUS::Version::Number;

#endif // WITH_EDITORONLY_DATA
};