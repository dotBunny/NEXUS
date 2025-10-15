// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "InputCoreTypes.h"
#include "NActorPoolSet.h"
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

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Actor Pools", DisplayName ="Stats Tracking",
		meta=(ToolTip="Should the stat group (NActorPools) be populated? (when available)"))
	bool bTrackStats = true;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Actor Pools", DisplayName ="Default Settings",
		meta=(ToolTip="The default settings applied to a created NActorPool when no settings are provided."))
	FNActorPoolSettings DefaultSettings;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Initialization", DisplayName="Always Create",
		meta=(ToolTip="Always create the outlined actor pool sets when a world is created."))
	TArray<TSoftObjectPtr<UNActorPoolSet>> AlwaysCreateSets;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Initialization", DisplayName = "Ignore World Prefixes", 
		meta=(Tooltop="Ignore attempting to auto create pools for worlds thats name starts with or is outlined."))
	TArray<FString> IgnoreWorldPrefixes;
};
