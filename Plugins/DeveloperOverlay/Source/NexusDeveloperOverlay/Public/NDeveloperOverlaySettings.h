// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NSettingsUtils.h"
#include "Macros/NSettingsMacros.h"
#include "NDeveloperOverlaySettings.generated.h"

UCLASS(Config=NexusGame, defaultconfig)
class NEXUSDEVELOPEROVERLAY_API UNDeveloperOverlaySettings : public UDeveloperSettings
{
	GENERATED_BODY()

	N_IMPLEMENT_SETTINGS(UNDeveloperOverlaySettings);

#if WITH_EDITOR

	virtual FName GetContainerName() const override { return FNSettingsUtils::GetContainerName(); }
	virtual FName GetCategoryName() const override {  return FNSettingsUtils::GetCategoryName();  }
	
	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Developer Overlay"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("Settings related to the Developer Overlay."));
		return SectionDescription;
	}

#endif	
	
public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Widget Settings", DisplayName ="Maximum Height",
		meta=(ToolTip="What is the maximum vertical height of any individual overlay widget?"))
	float DeveloperOverlayWidgetMaximumHeight = 500.f;
};
