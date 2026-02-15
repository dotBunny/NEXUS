// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NSettingsUtils.h"
#include "Macros/NSettingsMacros.h"
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
		const FText SectionDescription = FText::FromString(TEXT("Settings related to the Guardian plugin."));
		return SectionDescription;
	}
#endif // WITH_EDITOR	

	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Developer Subsystem", DisplayName ="Use Developer Subsystem?",
	meta=(ToolTip="Should the Developer Subsystem be created? It is required for UObject monitoring and you still must call UNDeveloperSubsystem::SetBaseline!"))
	bool bDeveloperSubsystemEnabled = false;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Developer Subsystem", DisplayName ="Warning Threshold",
	meta=(ToolTip="The number of added UObjects to a world when a warning should be thrown.", EditCondition="bDeveloperSubsystemEnabled"))
	int32 DeveloperObjectCountWarningThreshold = 25000;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Developer Subsystem", DisplayName ="Snapshot Threshold",
	meta=(ToolTip="The number of added UObjects to a world when a snapshot should be taken. This snapshot will be compared with later if the counts exceed the compare threshold.",
		EditCondition="bDeveloperSubsystemEnabled"))
	int32 DeveloperObjectCountSnapshotThreshold = 30000;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Developer Subsystem", DisplayName ="Save Capture",
		meta=(ToolTip="Should the capture data be saved to the project's logs folder? Saving the capture to disk is not required to compare, the snapshot will be stored in memory.",
			EditCondition="bDeveloperSubsystemEnabled"))
	bool bDeveloperObjectCountCaptureOutput = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Developer Subsystem", DisplayName ="Compare Threshold",
	meta=(ToolTip="The number of added UObjects to a world when a compare against the previous capture should be done.",
		EditCondition="bDeveloperSubsystemEnabled"))
	int32 DeveloperObjectCountCompareThreshold = 40000;
};
