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
		// We don't want to assume the RequiredAny as it is a condition of success and it might not be in use.
		
		if (!UniqueTags.HasTag(NWorldAssembly_Behavior_Unique))
		{
			UniqueTags.AppendTags(FGameplayTagContainer(NWorldAssembly_Behavior_Unique));
		}
		if (!BadNeighborsTags.HasTag(NWorldAssembly_Behavior_BadNeighbors))
		{
			BadNeighborsTags.AppendTags(FGameplayTagContainer(NWorldAssembly_Behavior_BadNeighbors));
		}
	}
	
	/** Tags that limit placement to a single item from the group; built-in Unique tag is auto-added. */
	UPROPERTY(EditAnywhere, DisplayName="Unique", meta = (Categories="NEXUS.WorldAssembly",
		ToolTip="When a tag group is flagged as unique it means that only one item can be placed in the graph from that group. NEXUS.WorldAssembly.Behavior.Unique is auto-added to this group. NOTE: If combined with RequiredAny, the MinimumCount will be ignored for all in the group, just ensuring that one is present."))
	FGameplayTagContainer UniqueTags;
	
	/** Tags that require at least one matching item to be placed during generation; built-in RequiredAny tag is auto-added. */
	UPROPERTY(EditAnywhere, DisplayName="Required (Any)", meta = (Categories="NEXUS.WorldAssembly",
		ToolTip="When a tag group is marked RequiredAny, it means that when generating we require something from that group to be placed in the graph.  NOTE: If combined with Unique, the MinimumCount will be ignored for all in the group, just ensuring that one is present."))
	FGameplayTagContainer RequiredAnyTags;
	
	/** Tags marking items that cannot be placed adjacent to one another; built-in BadNeighbors tag is auto-added. */
	UPROPERTY(EditAnywhere, DisplayName="Bad Neighbors", meta = (Categories="NEXUS.WorldAssembly",
		ToolTip="When a tag group is marked BadNeighbor, they cannot be placed beside each other."))
	FGameplayTagContainer BadNeighborsTags;
};