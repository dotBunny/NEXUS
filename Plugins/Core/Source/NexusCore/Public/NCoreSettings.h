// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once


#include "NSettingsUtils.h"
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

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Developer", DisplayName ="Object Monitoring",
	meta=(ToolTip="Monitor the number of UObjects and respond accordingly."))
	bool bDeveloperObjectMonitoring = false;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Developer", DisplayName ="Warning Threshold",
	meta=(ToolTip="The number of added UObjects to a world when a warning should be thrown."))
	int32 DeveloperObjectCountWarningThreshold = 2500;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Developer", DisplayName ="Capture Threshold",
	meta=(ToolTip="The number of added UObjects to a world when a capture should be taken."))
	int32 DeveloperObjectCountCaptureThreshold = 3000;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Developer", DisplayName ="Compare Threshold",
	meta=(ToolTip="The number of added UObjects to a world when a compare against the previous capture should be done."))
	int32 DeveloperObjectCountCompareThreshold = 4000;
};
