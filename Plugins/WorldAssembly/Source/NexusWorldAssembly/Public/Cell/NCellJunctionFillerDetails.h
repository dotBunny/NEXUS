// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "GameplayTagContainer.h"
#include "NArrayUtils.h"
#include "NCellJunctionFillerEntry.h"
#include "Collections/NGameplayTagCounterConstraint.h"

#include "NCellJunctionFillerDetails.generated.h"

USTRUCT(BlueprintType)
struct NEXUSWORLDASSEMBLY_API FNCellJunctionFillerDetails
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, DisplayName="Offset", Category="Object")
	FTransform Offset = FTransform::Identity;
	
	UPROPERTY(EditAnywhere, DisplayName="Assembly Tags", Category="Tagging",  meta = (Categories="NEXUS.WorldAssembly", ToolTip="Tags used to define behavior during the assembly process."))
	FGameplayTagContainer AssemblyTags;
	
	UPROPERTY(EditAnywhere, DisplayName="Added Context Tags", Category="Tagging", meta = (ToolTip="Tags added to the Context Tags collection moving forward for the graph, and future phases."))
	FGameplayTagContainer AddedContextTags;
	
	/** Tags required to be found in Context Tags for allowance to place this cell. **/
	UPROPERTY(EditAnywhere, DisplayName="Required Context Tags", Category="Tagging", meta = (ToolTip="Tags required to be found in Context Tags for allowance to place this cell."))
	FGameplayTagContainer RequiredContextTags;
	
	UPROPERTY(EditAnywhere, DisplayName="Tag Counter Constraints", Category="Tagging", meta = (ToolTip="TagCounter constraints that must pass for this cell to be included in cell selection. If a tag is constrained but not present in the current Tag Counter, a value of 0 is compared against."))
	TArray<FNGameplayTagCounterConstraint> TagCounterConstraints;
	
	UPROPERTY(EditAnywhere, DisplayName="Tag Counter Operations", Category="Tagging", meta = (ToolTip="Operations to apply to the TagCounter if the cell is placed/used."))
	TArray<FNGameplayTagCounterOperation> TagCounterOperations;
	
	/** 
	 * Relative weight for random selection during generation.
	 * @note Higher values increase the probability of this cell being chosen.
	 */
	UPROPERTY(EditAnywhere)
	int32 Weighting = 1;
	
	FNCellJunctionFillerDetails() = default;

	explicit FNCellJunctionFillerDetails(const FNCellJunctionFillerEntry& Filler)
	{
		Offset = Filler.Offset;
		AssemblyTags = Filler.AssemblyTags;
		AddedContextTags = Filler.AddedContextTags;
		RequiredContextTags = Filler.RequiredContextTags;
		TagCounterConstraints = Filler.TagCounterConstraints;
		TagCounterOperations = Filler.TagCounterOperations;
		Weighting = Filler.Weighting;
	}
	
	/**
	 * Tests whether this details structure matches a filler, disregarding the filler's Actor.
	 * @param Filler The filler to compare against; its Actor is intentionally ignored.
	 * @return true if every field copied by the FNCellJunctionFiller constructor matches structurally.
	 * @note Mirrors the fields copied in the explicit FNCellJunctionFiller constructor.
	 */
	bool Equals(const FNCellJunctionFillerEntry& Filler) const
	{
		return Offset.Equals(Filler.Offset)
		&& AssemblyTags == Filler.AssemblyTags
		&& AddedContextTags == Filler.AddedContextTags
		&& RequiredContextTags == Filler.RequiredContextTags
		&& FNArrayUtils::IsSameOrderedValues(TagCounterConstraints, Filler.TagCounterConstraints)
		&& FNArrayUtils::IsSameOrderedValues(TagCounterOperations, Filler.TagCounterOperations)
		&& Weighting == Filler.Weighting;
	}
};

/**
 * Per-junction collection of filler details.
 * @note Exists because UnrealHeaderTool forbids a container as the direct value type of a reflected TMap;
 *       interposing this USTRUCT lets UNCell::JunctionFillers persist a TArray of fillers per junction.
 */
USTRUCT()
struct NEXUSWORLDASSEMBLY_API FNCellJunctionFillerDetailsArray
{
	GENERATED_BODY()

	/** The filler details authored for the owning junction. */
	UPROPERTY(VisibleAnywhere)
	TArray<FNCellJunctionFillerDetails> Fillers;
};