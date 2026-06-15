// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "GameplayTagContainer.h"
#include "Collections/NGameplayTagCounterConstraint.h"

#include "NCellJunctionFillerEntry.generated.h"


/**
 * An authored candidate for filling an unconnected junction.
 *
 * Each entry names an actor to spawn along with a placement offset, eligibility constraints, and a selection weight.
 * At fill time the owning junction gates entries by their context-tag and tag-counter constraints, then picks one
 * weighted-at-random from the survivors (see UNCellJunctionComponent::Fill).
 */
USTRUCT(BlueprintType)
struct NEXUSWORLDASSEMBLY_API FNCellJunctionFillerEntry
{
	GENERATED_BODY()

	/** The actor to spawn when this entry is selected; must implement INCellJunctionFiller. */
	UPROPERTY(EditAnywhere, DisplayName="Actor", Category="Object", meta=(MustImplement="/Script/NexusWorldAssembly.NCellJunctionFiller"))
	TSubclassOf<AActor> Actor;

	/**
	 * Placement offset applied relative to the junction's frame: the location is rotated by the junction's orientation
	 * before being added, the rotation spins the actor in place at that spot, and the scale multiplies the actor's own scale.
	 */
	UPROPERTY(EditAnywhere, DisplayName="Offset", Category="Object")
	FTransform Offset = FTransform::Identity;
	
	/** Tags required to be found in Context Tags for allowance to place this cell. **/
	UPROPERTY(EditAnywhere, DisplayName="Required Context Tags", Category="Tagging", meta = (ToolTip="Tags required to be found in Context Tags for allowance to place this fill."))
	FGameplayTagContainer RequiredContextTags;
	
	/** TagCounter constraints that must pass for this entry to be eligible during fill selection (an absent tag compares as 0). */
	UPROPERTY(EditAnywhere, DisplayName="Tag Counter Constraints", Category="Tagging", meta = (ToolTip="TagCounter constraints that must pass for this entry to be included in fill selection. If a tag is constrained but not present in the current Tag Counter, a value of 0 is compared against."))
	TArray<FNGameplayTagCounterConstraint> TagCounterConstraints;
	
	/** 
	 * Relative weight for random selection during generation.
	 * @note Higher values increase the probability of this cell being chosen.
	 */
	UPROPERTY(EditAnywhere)
	int32 Weighting = 1;
};
