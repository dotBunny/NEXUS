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

	const FGameplayTagContainer& GetRequiredAnyTags() { return RequiredAnyTags; }
	const FGameplayTagContainer& GetUniqueTags() { return UniqueTags; }
	
	bool HasRequiredAnyTags() const
	{
		return !RequiredAnyTags.IsEmpty();
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
	
	void RemoveRequiredAnyTags(const FGameplayTagContainer& Tags)
	{
		RequiredAnyTags.RemoveTags(Tags);
	}
	
	void AppendRequiredAnyTags(const FGameplayTagContainer& Tags)
	{
		RequiredAnyTags.AppendTags(Tags);
	}
	
	bool HasAnyUniqueTags(const FGameplayTagContainer& Tags) const
	{
		return UniqueTags.HasAny(Tags);
	}
	
	bool HasAnyRequiredAnyTags(const FGameplayTagContainer& Tags) const
	{
		return RequiredAnyTags.HasAny(Tags);
	}
	
	bool HasAllRequiredAnyTags(const FGameplayTagContainer& Tags) const
	{
		return RequiredAnyTags.HasAllExact(Tags);
	}
	
	FGameplayTagContainer FilterUniqueTags(const FGameplayTagContainer& Tags) const
	{
		return UniqueTags.Filter(Tags);
	}
	
	FGameplayTagContainer FilterRequiredAnyTags(const FGameplayTagContainer& Tags) const
	{
		return RequiredAnyTags.Filter(Tags);
	}	
	
	void AppendTags(const FNTissueTagGroups& OtherTagGroup)
	{
		UniqueTags.AppendTags(OtherTagGroup.UniqueTags);
		RequiredAnyTags.AppendTags(OtherTagGroup.RequiredAnyTags);
	}
	
	void EnsureBaseTags()
	{
		if (!UniqueTags.HasTag(NWorldAssembly_BuiltIn_Unique))
		{
			UniqueTags.AppendTags(FGameplayTagContainer(NWorldAssembly_BuiltIn_Unique));
		}
		if (!RequiredAnyTags.HasTag(NWorldAssembly_BuiltIn_RequiredAny))
		{
			RequiredAnyTags.AppendTags(FGameplayTagContainer(NWorldAssembly_BuiltIn_RequiredAny));
		}
	}
	
protected:
	UPROPERTY(EditAnywhere, DisplayName="Unique", meta = (Categories="NEXUS.WorldAssembly", 
		ToolTip="When a tag group is flagged as unique it means that only one item can be placed in the graph from that group. NEXUS.WorldAssembly.BuiltIn.Unique is auto-added to this group. NOTE: If combined with RequiredAny, the MinimumCount will be ignored for all in the group, just ensuring that one is present."))
	FGameplayTagContainer UniqueTags;
	
	UPROPERTY(EditAnywhere, DisplayName="Required (Any)", meta = (Categories="NEXUS.WorldAssembly", 
		ToolTip="When a tag group is marked RequiredAny, it means that when generating we require something from that group to be placed in the graph. NEXUS.WorldAssembly.BuiltIn.RequiredAny is auto-added to this group. NOTE: If combined with Unique, the MinimumCount will be ignored for all in the group, just ensuring that one is present."))
	FGameplayTagContainer RequiredAnyTags;
};