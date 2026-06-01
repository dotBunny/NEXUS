// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameplayTagContainer.h"
#include "NWorldAssemblyGameplayTags.h"
#include "Engine/DataAsset.h"
#include "NTissueTagGroups.generated.h"

/**
 * Pairs the two gameplay tag sets that drive cell placement: "unique" tags that cap an item to a single
 * placement in the graph, and "required (any)" tags that force at least one matching item to be placed.
 *
 * Both sets default to their built-in tags (see EnsureBaseTags) so a freshly created group always carries
 * the framework's baseline behavior.
 */
USTRUCT()
struct FNTissueTagGroups
{
	GENERATED_BODY()

	/** @return The "required (any)" tag set. */
	const FGameplayTagContainer& GetRequiredAnyTags() { return RequiredAnyTags; }
	/** @return The "unique" tag set. */
	const FGameplayTagContainer& GetUniqueTags() { return UniqueTags; }
	
	const FGameplayTagContainer& GetBadNeighborsTags() { return BadNeighborsTags; }

	/** @return true if any "required (any)" tags are present. */
	bool HasRequiredAnyTags() const
	{
		return !RequiredAnyTags.IsEmpty();
	}

	/** @return true if any "unique" tags are present. */
	bool HasUniqueTags() const
	{
		return !UniqueTags.IsEmpty();
	}
	
	bool HasBadNeighborsTags() const
	{
		return !BadNeighborsTags.IsEmpty();
	}


	/** Add the supplied tags to the "unique" set. */
	void AppendUniqueTags(const FGameplayTagContainer& Tags)
	{
		UniqueTags.AppendTags(Tags);
	}
	
	void AppendBadNeighborsTags(const FGameplayTagContainer& Tags)
	{
		BadNeighborsTags.AppendTags(Tags);
	}

	/** Remove the supplied tags from the "unique" set. */
	void RemoveUniqueTags(const FGameplayTagContainer& Tags)
	{
		UniqueTags.RemoveTags(Tags);
	}

	/** Remove the supplied tags from the "required (any)" set. */
	void RemoveRequiredAnyTags(const FGameplayTagContainer& Tags)
	{
		RequiredAnyTags.RemoveTags(Tags);
	}
	
	void RemoveBadNeighborsTags(const FGameplayTagContainer& Tags)
	{
		BadNeighborsTags.RemoveTags(Tags);
	}

	/** Add the supplied tags to the "required (any)" set. */
	void AppendRequiredAnyTags(const FGameplayTagContainer& Tags)
	{
		RequiredAnyTags.AppendTags(Tags);
	}

	/** @return true if the "unique" set contains any of the supplied tags. */
	bool HasAnyUniqueTags(const FGameplayTagContainer& Tags) const
	{
		return UniqueTags.HasAny(Tags);
	}

	/** @return true if the "required (any)" set contains any of the supplied tags. */
	bool HasAnyRequiredAnyTags(const FGameplayTagContainer& Tags) const
	{
		return RequiredAnyTags.HasAny(Tags);
	}

	bool HasAnyBadNeighborsTags(const FGameplayTagContainer& Tags) const
	{
		return BadNeighborsTags.HasAny(Tags);
	}

	
	/** @return true if the "required (any)" set contains an exact match for every supplied tag. */
	bool HasAllRequiredAnyTags(const FGameplayTagContainer& Tags) const
	{
		return RequiredAnyTags.HasAllExact(Tags);
	}
	
	bool HasAllBadNeighborsTags(const FGameplayTagContainer& Tags) const
	{
		return BadNeighborsTags.HasAllExact(Tags);
	}

	/** @return The subset of the "unique" set that matches the supplied filter tags. */
	FGameplayTagContainer FilterUniqueTags(const FGameplayTagContainer& Tags) const
	{
		return UniqueTags.Filter(Tags);
	}

	/** @return The subset of the "required (any)" set that matches the supplied filter tags. */
	FGameplayTagContainer FilterRequiredAnyTags(const FGameplayTagContainer& Tags) const
	{
		return RequiredAnyTags.Filter(Tags);
	}
	
	FGameplayTagContainer FilterBadNeighborsTags(const FGameplayTagContainer& Tags) const
	{
		return BadNeighborsTags.Filter(Tags);
	}

	/** Merge both tag sets from another group into this one. */
	void AppendTags(const FNTissueTagGroups& OtherTagGroup)
	{
		UniqueTags.AppendTags(OtherTagGroup.UniqueTags);
		RequiredAnyTags.AppendTags(OtherTagGroup.RequiredAnyTags);
		BadNeighborsTags.AppendTags(OtherTagGroup.BadNeighborsTags);
	}

	/** Ensure each set contains its built-in baseline tag, adding it if missing. */
	void EnsureBaseTags()
	{
		if (!UniqueTags.HasTag(NWorldAssembly_Behavior_Unique))
		{
			UniqueTags.AppendTags(FGameplayTagContainer(NWorldAssembly_Behavior_Unique));
		}
		if (!RequiredAnyTags.HasTag(NWorldAssembly_Behavior_RequiredAny))
		{
			RequiredAnyTags.AppendTags(FGameplayTagContainer(NWorldAssembly_Behavior_RequiredAny));
		}
		if (!BadNeighborsTags.HasTag(NWorldAssembly_Behavior_BadNeighbors))
		{
			BadNeighborsTags.AppendTags(FGameplayTagContainer(NWorldAssembly_Behavior_BadNeighbors));
		}
	}
	
protected:
	/** Tags that limit placement to a single item from the group; built-in Unique tag is auto-added. */
	UPROPERTY(EditAnywhere, DisplayName="Unique", meta = (Categories="NEXUS.WorldAssembly",
		ToolTip="When a tag group is flagged as unique it means that only one item can be placed in the graph from that group. NEXUS.WorldAssembly.BuiltIn.Unique is auto-added to this group. NOTE: If combined with RequiredAny, the MinimumCount will be ignored for all in the group, just ensuring that one is present."))
	FGameplayTagContainer UniqueTags;
	
	/** Tags that require at least one matching item to be placed during generation; built-in RequiredAny tag is auto-added. */
	UPROPERTY(EditAnywhere, DisplayName="Required (Any)", meta = (Categories="NEXUS.WorldAssembly",
		ToolTip="When a tag group is marked RequiredAny, it means that when generating we require something from that group to be placed in the graph. NEXUS.WorldAssembly.BuiltIn.RequiredAny is auto-added to this group. NOTE: If combined with Unique, the MinimumCount will be ignored for all in the group, just ensuring that one is present."))
	FGameplayTagContainer RequiredAnyTags;
	
	UPROPERTY(EditAnywhere, DisplayName="Bad Neighbors", meta = (Categories="NEXUS.WorldAssembly",
		ToolTip="When a tag group is marked BadNeighbor, they cannot be placed beside each other."))
	FGameplayTagContainer BadNeighborsTags;
};