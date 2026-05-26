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

	const FGameplayTagContainer& GetRequiredTags() { return RequiredTags; }
	const FGameplayTagContainer& GetUniqueTags() { return UniqueTags; }
	
	bool HasRequiredTags() const
	{
		return !RequiredTags.IsEmpty();
	}
	
	bool HasUniqueTags() const
	{
		return !UniqueTags.IsEmpty();
	}
	
	
	void AppendUniqueTags(const FGameplayTagContainer& Tags)
	{
		UniqueTags.AppendTags(Tags);
	}
	
	void RemoveUniqueTags(const FGameplayTagContainer& Tags)
	{
		UniqueTags.RemoveTags(Tags);
	}
	
	void RemoveRequiredTags(const FGameplayTagContainer& Tags)
	{
		RequiredTags.RemoveTags(Tags);
	}
	
	void AppendRequiredTags(const FGameplayTagContainer& Tags)
	{
		RequiredTags.AppendTags(Tags);
	}
	
	bool HasAnyUniqueTags(const FGameplayTagContainer& Tags) const
	{
		return UniqueTags.HasAny(Tags);
	}
	
	bool HasAnyRequiredTags(const FGameplayTagContainer& Tags) const
	{
		return RequiredTags.HasAny(Tags);
	}
	
	bool HasAllRequiredTags(const FGameplayTagContainer& Tags) const
	{
		return RequiredTags.HasAllExact(Tags);
	}
	
	FGameplayTagContainer FilterUniqueTags(const FGameplayTagContainer& Tags) const
	{
		return UniqueTags.Filter(Tags);
	}
	
	FGameplayTagContainer FilterRequiredTags(const FGameplayTagContainer& Tags) const
	{
		return RequiredTags.Filter(Tags);
	}	
	
	void AppendTags(const FNTissueTagGroups& OtherTagGroup)
	{
		UniqueTags.AppendTags(OtherTagGroup.UniqueTags);
		RequiredTags.AppendTags(OtherTagGroup.RequiredTags);
	}
	
	void EnsureBaseTags()
	{
		if (!UniqueTags.HasTag(NWorldAssembly_BuiltIn_Unique))
		{
			UniqueTags.AppendTags(FGameplayTagContainer(NWorldAssembly_BuiltIn_Unique));
		}
		if (!RequiredTags.HasTag(NWorldAssembly_BuiltIn_Required))
		{
			RequiredTags.AppendTags(FGameplayTagContainer(NWorldAssembly_BuiltIn_Required));
		}
	}
	
protected:
	UPROPERTY(EditAnywhere, DisplayName="Unique", meta = (Categories="NEXUS.WorldAssembly", 
		ToolTip="When a tag group is flagged as unique it means that only one item can be placed in the graph from that group. NEXUS.WorldAssembly.BuiltIn.Unique is auto-added to this group. NOTE: If combined with Must Have, the MinimumCount will be ignored for all in the group, just ensuring that one is present."))
	FGameplayTagContainer UniqueTags;
	
	UPROPERTY(EditAnywhere, DisplayName="Required", meta = (Categories="NEXUS.WorldAssembly", 
		ToolTip="When a tag group is marked Must Have, it means that when generating we require something from that group to be placed in the graph. NEXUS.WorldAssembly.BuiltIn.Required is auto-added to this group. NOTE: If combined with Unique, the MinimumCount will be ignored for all in the group, just ensuring that one is present."))
	FGameplayTagContainer RequiredTags;
};