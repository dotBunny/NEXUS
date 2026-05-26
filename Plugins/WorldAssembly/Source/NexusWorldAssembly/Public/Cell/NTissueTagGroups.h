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
	
	void EnsureBaseTags()
	{
		if (!UniqueTags.HasTag(NWorldAssembly_BuiltIn_Unique))
		{
			UniqueTags.AppendTags(FGameplayTagContainer(NWorldAssembly_BuiltIn_Unique));
		}
		if (!MustHaveTags.HasTag(NWorldAssembly_BuiltIn_MustHave))
		{
			MustHaveTags.AppendTags(FGameplayTagContainer(NWorldAssembly_BuiltIn_MustHave));
		}
	}
	
protected:
	UPROPERTY(EditAnywhere, DisplayName="Unique", meta = (Categories="NEXUS.WorldAssembly", 
		ToolTip="When a tag group is flagged as unique it means that only one item can be placed in the graph from that group. NEXUS.WorldAssembly.BuiltIn.Unique is auto-added to this group. NOTE: If combined with Must Have, the MinimumCount will be ignored for all in the group, just ensuring that one is present."))
	FGameplayTagContainer UniqueTags;
	
	UPROPERTY(EditAnywhere, DisplayName="Must Have", meta = (Categories="NEXUS.WorldAssembly", 
		ToolTip="When a tag group is marked Must Have, it means that when generating we require something from that group to be placed in the graph. NEXUS.WorldAssembly.BuiltIn.MustHave is auto-added to this group. NOTE: If combined with Unique, the MinimumCount will be ignored for all in the group, just ensuring that one is present."))
	FGameplayTagContainer MustHaveTags;
};