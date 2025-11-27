// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NProcGenDeveloperOverlayWidget.h"
#include "NSettingsUtils.h"
#include "Macros/NSettingsMacros.h"
#include "NProcGenSettings.generated.h"

UCLASS(Config=NexusGame, defaultconfig)
class NEXUSPROCGEN_API UNProcGenSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	N_IMPLEMENT_SETTINGS(UNProcGenSettings);

#if WITH_EDITOR

	virtual FName GetContainerName() const override { return FNSettingsUtils::GetContainerName(); }
	virtual FName GetCategoryName() const override {  return FNSettingsUtils::GetCategoryName();  }
	
	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Procedural Generation"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("Settings related to ProcGen."));
		return SectionDescription;
	}

#endif	

	//TODO: junctions are based on the mid-point, and draw from that dunno if we need to rethink that drawing because of voxel.
public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Generation Settings", DisplayName ="Unit Size",
		meta=(ToolTip="What is the base unit sized when operating on our grid?"))
	FVector UnitSize = FVector(50.f, 50.f, 50.f);

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Generation Settings", DisplayName ="Player Size",
		meta=(ToolTip="What is the size of the player's collider?"))
	FVector PlayerSize = FVector(72.f, 184.f, 72.f);
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UNProcGenDeveloperOverlayWidget> DeveloperOverlayWidget;
};