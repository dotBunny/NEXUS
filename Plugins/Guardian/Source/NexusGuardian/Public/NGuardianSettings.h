// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NSettingsUtils.h"
#include "Macros/NSettingsMacros.h"
#include "Types/NBuildConfigurationAvailability.h"
#include "NGuardianSettings.generated.h"

/**
 * Project-wide configuration for the NexusGuardian subsystem.
 *
 * Exposes which build configurations the subsystem is active in, plus the three thresholds that
 * drive the warn/snapshot/compare ladder as UObject counts grow.
 * @see <a href="https://nexus-framework.com/docs/plugins/guardian/project-settings//">UNGuardianSettings</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "Guardian Settings", Config=NexusGame, defaultconfig)
class UNGuardianSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	N_SETTINGS_BASE(UNGuardianSettings, "Guardian", "Settings related to the Guardian subsystem.")

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Subsystem", DisplayName="Build Availability", meta=(Bitmask,BitmaskEnum="/Script/NexusCore.ENBuildConfigurationAvailability",ToolTip="Which build configurations the Guardian subsystem is active in (bitmask of ENBuildConfigurationAvailability)."))
	uint8 BuildAvailability = N_BUILD_CONFIGURATION_AVAILABILITY_ALL_NOT_SHIPPING;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Subsystem", DisplayName ="Tick Rate",
		meta=(ToolTip="How often should the system poll the object count? and respond.",
			EditCondition="BuildAvailability!=0", ClampMin="0.0", UIMin="0.0", ClampMin="0.0", UIMin="0.0", ClampMax="300.0", UIMax="300.0", Units="s"))
	float TickRate = 1.0f;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Subsystem", DisplayName ="Save Capture",
		meta=(ToolTip="Should the capture data be saved to the project's logs folder? Saving the capture to disk is not required to compare, the snapshot will be stored in memory.",
			EditCondition="BuildAvailability!=0"))
	bool bObjectCountCaptureOutput = false;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Baseline", DisplayName ="Auto Baseline",
		meta=(ToolTip="Automatically establish the UObject count baseline when a world begins play. Disable if you want to call SetBaseline() manually at a more representative point in your game's lifecycle.",
			EditCondition="BuildAvailability!=0"))
	bool bAutoBaseline = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Baseline", DisplayName ="Auto Baseline Delay",
		meta=(ToolTip="Seconds to wait after world begin play before capturing the auto baseline. A short delay lets transient startup objects settle so the baseline reflects steady-state object counts.",
			EditCondition="BuildAvailability!=0 && bAutoBaseline", ClampMin="0.0", UIMin="0.0", Units="s"))
	float AutoBaselineDelay = 15.f;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Thresholds", DisplayName ="Warning Threshold",
	meta=(ToolTip="The number of added UObjects to a world when a warning should be thrown.", EditCondition="BuildAvailability!=0", ClampMin="1", UIMin="1"))
	int32 ObjectCountWarningThreshold = 25000;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Thresholds", DisplayName ="Snapshot Threshold",
	meta=(ToolTip="The number of added UObjects to a world when a snapshot should be taken. This snapshot will be compared with later if the counts exceed the compare threshold.",
		EditCondition="BuildAvailability!=0", ClampMin="2", UIMin="2"))
	int32 ObjectCountSnapshotThreshold = 30000;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Thresholds", DisplayName ="Compare Threshold",
		meta=(ToolTip="The number of added UObjects to a world when a compare against the previous capture should be done.",
			EditCondition="BuildAvailability!=0", ClampMin="3", UIMin="3"))
	int32 ObjectCountCompareThreshold = 40000;
};
