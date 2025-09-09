// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once


#include "NSettingsUtils.h"
#include "Developer/NDeveloperLibrary.h"
#include "Macros/NSettingsMacros.h"
#include "NCoreSettings.generated.h"

UCLASS(Config=NexusGame, defaultconfig)
class NEXUSCORE_API UNCoreSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	N_IMPLEMENT_SETTINGS(UNCoreSettings);

public:
#if WITH_EDITOR	
	virtual FName GetContainerName() const override { return FNSettingsUtils::GetContainerName(); }
	virtual FName GetCategoryName() const override {  return FNSettingsUtils::GetCategoryName();  }
	

	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Core"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("Settings related to the core plugin."));
		return SectionDescription;
	}
#endif	

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Developer Subsystem", DisplayName ="Object Monitoring",
	meta=(ToolTip="Monitor the number of UObjects and respond accordingly. You still must call UNDeveloperSubsystem::SetBaseline!"))
	bool bDeveloperObjectMonitoring = false;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Developer Subsystem", DisplayName ="Warning Threshold",
	meta=(ToolTip="The number of added UObjects to a world when a warning should be thrown.", EditCondition="bDeveloperObjectMonitoring"))
	int32 DeveloperObjectCountWarningThreshold = 25000;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Developer Subsystem", DisplayName ="Snapshot Threshold",
	meta=(ToolTip="The number of added UObjects to a world when a snapshot should be taken. This snapshot will be compared with later if the counts exceed the compare threshold.",
		EditCondition="bDeveloperObjectMonitoring"))
	int32 DeveloperObjectCountSnapshotThreshold = 30000;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Developer Subsystem", DisplayName ="Save Capture",
		meta=(ToolTip="Should the capture data be saved to the project's logs folder? Saving the capture to disk is not required to compare, the snapshot will be stored in memory.",
			EditCondition="bDeveloperObjectMonitoring"))
	bool bDeveloperObjectCountCaptureOutput = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Developer Subsystem", DisplayName ="Compare Threshold",
	meta=(ToolTip="The number of added UObjects to a world when a compare against the previous capture should be done.",
		EditCondition="bDeveloperObjectMonitoring"))
	int32 DeveloperObjectCountCompareThreshold = 40000;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Leak Check", DisplayName ="Duration", meta=(ToolTip="How long should the leak check run doing its capture."))
	float LeakCheckTime = 30.f;
};
