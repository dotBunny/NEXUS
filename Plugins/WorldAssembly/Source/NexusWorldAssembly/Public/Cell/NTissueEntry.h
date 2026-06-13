// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "GameplayTagContainer.h"
#include "NCell.h"
#include "Collections/NGameplayTagCounterConstraint.h"
#include "Types/NCardinalDirection.h"

#include "NTissueEntry.generated.h"

/**
 * A single cell entry within a UNTissue palette, carrying the generation constraints
 * (counts, weighting, graph position rules) that control how often and where the cell
 * is allowed to appear in the generated FNAssemblyGraph.
 */
USTRUCT()
struct NEXUSWORLDASSEMBLY_API FNTissueEntry
{
	GENERATED_BODY()
	
	/** Tags used to define behavior during the assembly process. */
	UPROPERTY(EditAnywhere, DisplayName="Assembly Tags", Category="Tagging",  meta = (Categories="NEXUS.WorldAssembly", ToolTip="Tags used to define behavior during the assembly process."))
	FGameplayTagContainer AssemblyTags;

	/** Tags added to the Context Tags collection moving forward for the graph, and future phases. */
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
	 * A minimum number of times this cell must be used in the generated FNAssemblyGraph for the graph to validate.
	 * Enforced in FNVirtualOrganContext::CheckGraph against the cell's placed instance count.
	 * @note A value of 0 indicates no minimum constraint.
	 * @remark The minimum is skipped for cells whose assembly tags name a group that is both Unique and RequiredAny
	 *         (the RequiredAny group check governs success instead), and for cells whose MinimumCount exceeds a
	 *         positive MaximumCount (unsatisfiable). Also used to determine the Unique early out (MinimumCount == 1
	 *         && MaximumCount == 1).
	 */
	UPROPERTY(EditAnywhere, meta=(ClampMin=0, UIMin=0))
	int32 MinimumCount = 0;

	/**
	 * The maximum number of times this cell can be used in the generated FNAssemblyGraph.
	 * @note A value of 0 indicates no maximum constraint (unlimited usage). To stop a cell from being generated,
	 *       remove it from the tissue rather than setting a count.
	 */
	UPROPERTY(EditAnywhere, meta=(ClampMin=0, UIMin=0))
	int32 MaximumCount = 0;
	
	/**
	 * The minimum number of cell links away this cell must be to be used again.
	 * @note A value of 0 indicates no constraint.
	 */
	UPROPERTY(EditAnywhere,meta=(ClampMin=0, UIMin=0))
	int32 MinimumNodeDistance = 1;
	
	/**
	* The minimum graph depth at which this cell may be used, as a 1-based NodeDepth.
	* The start cell is depth 1, its direct neighbors depth 2, etc. A value of N first allows the cell at depth N.
	* @note A value of 0 indicates no constraint (a value of 1 is the start cell and is likewise unconstrained).
	*/
	UPROPERTY(EditAnywhere,meta=(ClampMin=0, UIMin=0))
	int32 MinimumNodeDepth = 0;

	/**
	* The maximum graph depth at which this cell may still be used, as a 1-based NodeDepth.
	* The start cell is depth 1, its direct neighbors depth 2, etc. A value of N allows the cell up to depth N;
	* a value of 1 restricts the cell to the start cell only.
	* @note A value of 0 indicates no constraint.
	*/
	UPROPERTY(EditAnywhere,meta=(ClampMin=0, UIMin=0))
	int32 MaximumNodeDepth = 0;

	/**
	 * When true, this cell may only be placed toward DirectionConstraint relative to the organ's start point,
	 * limiting it to candidates whose compass bearing from the start lands within the assembly direction tolerance.
	 */
	UPROPERTY(EditAnywhere)
	bool bHasDirectionConstraint = false;

	/**
	 * The compass heading, measured from the organ's start point out to the candidate's placement, this cell is
	 * restricted to while bHasDirectionConstraint is set.
	 * @note Enforced within the project/operation Direction Tolerance (degrees +/-) during cell filtering.
	 */
	UPROPERTY(EditAnywhere, meta=(EditCondition=bHasDirectionConstraint))
	ENCardinalDirection DirectionConstraint = ENCardinalDirection::North;
	
	/** 
	 * Relative weight for random selection during generation.
	 * @note Higher values increase the probability of this cell being chosen.
	 */
	UPROPERTY(EditAnywhere)
	int32 Weighting = 1;
	
	/** The cell asset this entry refers to. */
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UNCell> Cell;
};
