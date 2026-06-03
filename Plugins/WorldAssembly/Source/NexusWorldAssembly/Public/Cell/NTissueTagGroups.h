// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameplayTagContainer.h"
#include "NWorldAssemblyGameplayTags.h"
#include "Engine/DataAsset.h"
#include "NTissueTagGroups.generated.h"

/**
 * Groups the three gameplay tag sets that drive cell placement: "unique" tags that cap an item to a single
 * placement in the graph, "required (any)" tags that force at least one matching item to be placed, and
 * "bad neighbors" tags that stop members of a group being placed beside one another.
 *
 * These sets are empty on construction; baseline tags are not applied automatically. EnsureBaseTags() seeds
 * the framework's built-in tag into the Unique and BadNeighbors sets only (RequiredAny is intentionally left
 * alone, since requiring its baseline would impose an unsatisfiable success condition).
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

	/** Ensure the Unique and BadNeighbors sets contain their built-in baseline tag, adding it if missing. */
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
		ToolTip="When a tag group is flagged as unique it means that only one item can be placed in the graph from that group. NEXUS.WorldAssembly.Behavior.Unique is auto-added to this group. NOTE: If combined with RequiredAny, per-cell MinimumCount does not apply to the group; success only requires that one member is present."))
	FGameplayTagContainer UniqueTags;
	
	/** Tags that require at least one matching item to be placed during generation; built-in RequiredAny tag is auto-added. */
	UPROPERTY(EditAnywhere, DisplayName="Required (Any)", meta = (Categories="NEXUS.WorldAssembly",
		ToolTip="When a tag group is marked RequiredAny, it means that when generating we require something from that group to be placed in the graph.  NOTE: If combined with Unique, per-cell MinimumCount does not apply to the group; success only requires that one member is present."))
	FGameplayTagContainer RequiredAnyTags;
	
	/** Tags marking items that cannot be placed adjacent to one another; built-in BadNeighbors tag is auto-added. */
	UPROPERTY(EditAnywhere, DisplayName="Bad Neighbors", meta = (Categories="NEXUS.WorldAssembly",
		ToolTip="When a tag group is marked BadNeighbor, they cannot be placed beside each other."))
	FGameplayTagContainer BadNeighborsTags;
};