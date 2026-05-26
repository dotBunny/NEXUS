// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameplayTagContainer.h"
#include "NWorldAssemblyGameplayTags.h"
#include "Engine/DataAsset.h"
#include "NTissueTagGroups.generated.h"

USTRUCT()
struct FNTissueTagGroups
{
	GENERATED_BODY()

	const FGameplayTagContainer& GetMustHaveTags() { return MustHaveTags; }
	const FGameplayTagContainer& GetUniqueTags() { return UniqueTags; }
	
	bool HasAnyUniqueTags(const FGameplayTagContainer& Tags) const
	{
		return UniqueTags.HasAny(Tags);
	}
	
	bool HasAnyMustHaveTags(const FGameplayTagContainer& Tags) const
	{
		return MustHaveTags.HasAny(Tags);
	}
	
	FGameplayTagContainer FilterUniqueTags(const FGameplayTagContainer& Tags) const
	{
		return UniqueTags.Filter(Tags);
	}
	
	FGameplayTagContainer FilterMustHaveTags(const FGameplayTagContainer& Tags) const
	{
		return MustHaveTags.Filter(Tags);
	}	
	
	void AppendTags(const FNTissueTagGroups& OtherTagGroup)
	{
		UniqueTags.AppendTags(OtherTagGroup.UniqueTags);
		MustHaveTags.AppendTags(OtherTagGroup.MustHaveTags);
	}
	
protected:
	UPROPERTY(EditAnywhere, DisplayName="Unique", meta = (Categories="NEXUS.WorldAssembly"))
	FGameplayTagContainer UniqueTags = FGameplayTagContainer(NWorldAssembly_BuiltIn_Unique);
	
	UPROPERTY(EditAnywhere, DisplayName="Must Have", meta = (Categories="NEXUS.WorldAssembly"))
	FGameplayTagContainer MustHaveTags = FGameplayTagContainer(NWorldAssembly_BuiltIn_MustHave);
};