// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NSettingsUtils.h"
#include "Macros/NSettingsMacros.h"
#include "Types/NBuildConfigurationAvailability.h"
#include "NGuardianSettings.generated.h"

UCLASS(ClassGroup = "NEXUS", DisplayName = "Guardian Settings", Config=NexusGame, defaultconfig)
class UNGuardianSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	N_IMPLEMENT_SETTINGS(UNGuardianSettings);

public:
#if WITH_EDITOR	
	virtual FName GetContainerName() const override { return FNSettingsUtils::GetContainerName(); }
	virtual FName GetCategoryName() const override {  return FNSettingsUtils::GetCategoryName();  }

	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Guardian"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("Settings related to the Guardian subsystem."));
		return SectionDescription;
	}
#endif // WITH_EDITOR	

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Subsystem", DisplayName="Build Availability", meta=(Bitmask,BitmaskEnum="/Script/NexusCore.ENBuildConfigurationAvailability"))
	uint8 BuildAvailability = N_BUILD_CONFIGURATION_AVAILABILITY_ALL_NOT_SHIPPING;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Subsystem", DisplayName ="Save Capture",
		meta=(ToolTip="Should the capture data be saved to the project's logs folder? Saving the capture to disk is not required to compare, the snapshot will be stored in memory.",
			EditCondition="BuildAvailability!=0"))
	bool bObjectCountCaptureOutput = false;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Thresholds", DisplayName ="Warning Threshold",
	meta=(ToolTip="The number of added UObjects to a world when a warning should be thrown.", EditCondition="BuildAvailability!=0"))
	int32 ObjectCountWarningThreshold = 25000;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Thresholds", DisplayName ="Snapshot Threshold",
	meta=(ToolTip="The number of added UObjects to a world when a snapshot should be taken. This snapshot will be compared with later if the counts exceed the compare threshold.",
		EditCondition="BuildAvailability!=0"))
	int32 ObjectCountSnapshotThreshold = 30000;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Thresholds", DisplayName ="Compare Threshold",
		meta=(ToolTip="The number of added UObjects to a world when a compare against the previous capture should be done.",
			EditCondition="BuildAvailability!=0"))
	int32 ObjectCountCompareThreshold = 40000;
};
