// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameplayTagContainer.h"
#include "Collections/NGameplayTagCounterConstraint.h"
#include "Templates/SubclassOf.h"

#include "NCellJunctionConnectorEntry.generated.h"

/**
 * An authored candidate for connecting a junction that the connector pass paired with another cell's junction.
 *
 * The connector counterpart to FNCellJunctionFillerEntry, and selected the same way: entries are gated by their
 * context-tag and tag-counter constraints against the owning cell's assembly state, then one is picked
 * weighted-at-random from the survivors. Authored on UNCellJunctionComponent for a single junction and on
 * UNOrganComponent for every junction the organ places; a junction's own list wins over its organ's, and both win
 * over the project-wide default connector.
 *
 * @see <a href="https://nexus-framework.com/docs/plugins/world-assembly/types/cell-junction-connector-entry/">FNCellJunctionConnectorEntry</a>
 */
USTRUCT(BlueprintType)
struct NEXUSWORLDASSEMBLY_API FNCellJunctionConnectorEntry
{
	GENERATED_BODY()

	/** The actor to spawn when this entry is selected. Expected to implement INCellJunctionConnector. */
	UPROPERTY(EditAnywhere, DisplayName="Actor", Category="Object", meta=(MustImplement="/Script/NexusWorldAssembly.NCellJunctionConnector"))
	TSubclassOf<AActor> Actor;

	/**
	 * Placement offset applied relative to the start junction's frame: the location is rotated by that junction's
	 * orientation before being added, the rotation spins the actor in place at that spot, and the scale multiplies
	 * the actor's own scale.
	 */
	UPROPERTY(EditAnywhere, DisplayName="Offset", Category="Object")
	FTransform Offset = FTransform::Identity;

	/** Tags required to be found in Context Tags for this entry to be eligible. */
	UPROPERTY(EditAnywhere, DisplayName="Required Context Tags", Category="Tagging", meta = (ToolTip="Tags required to be found in Context Tags for allowance to use this connector."))
	FGameplayTagContainer RequiredContextTags;

	/** TagCounter constraints that must pass for this entry to be eligible during connector selection (an absent tag compares as 0). */
	UPROPERTY(EditAnywhere, DisplayName="Tag Counter Constraints", Category="Tagging", meta = (TitleProperty="{Tag}", ToolTip="TagCounter constraints that must pass for this entry to be included in connector selection. If a tag is constrained but not present in the current Tag Counter, a value of 0 is compared against."))
	TArray<FNGameplayTagCounterConstraint> TagCounterConstraints;

	/**
	 * Relative weight for random selection among eligible entries.
	 * @note Higher values increase the probability of this connector being chosen.
	 */
	UPROPERTY(EditAnywhere)
	int32 Weighting = 1;
};
