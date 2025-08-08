﻿// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "InputCoreTypes.h"
#include "NActorPoolSettings.h"
#include "NSettingsUtils.h"
#include "Macros/NSettingsMacros.h"
#include "NActorPoolsSettings.generated.h"

UCLASS(Config=NexusGame, defaultconfig)
class NEXUSACTORPOOLS_API UNActorPoolsSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	N_IMPLEMENT_SETTINGS(UNActorPoolsSettings);

public:
#if WITH_EDITOR	
	virtual FName GetContainerName() const override { return FNSettingsUtils::GetContainerName(); }
	virtual FName GetCategoryName() const override {  return FNSettingsUtils::GetCategoryName();  }
	

	virtual FText GetSectionText() const override
	{
		const FText SectionText =  FText::FromString(TEXT("Actor Pools"));
		return SectionText;
	}
	virtual FText GetSectionDescription() const override
	{
		const FText SectionDescription = FText::FromString(TEXT("Settings related to the Actor Pools."));
		return SectionDescription;
	}
#endif	

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Actor Pools", DisplayName ="Default Settings",
		meta=(ToolTip="The default settings applied to a created NActorPool when no settings are provided."))
	FNActorPoolSettings DefaultSettings;
};
