// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "GameplayTagContainer.h"
#include "Cell/NCell.h"
#include "Cell/NCellJunctionDetails.h"
#include "Cell/NCellRootDetails.h"
#include "Cell/NTissueTagGroups.h"
#include "Collections/NGameplayTagCounterConstraint.h"
#include "Types/NCardinalDirection.h"

/**
 * A lightweight summary of a cell's tagging, used to classify it (starter/finisher eligibility) during virtual organ processing.
 */
struct NEXUSWORLDASSEMBLY_API FNVirtualCellDataSummary
{
	/** Tissue tag groups collected for the cell. */
	FNTissueTagGroups GroupTags;

	/** true if any processed cell carries a starter tag. */
	bool bFoundStarterTagged = false;

	/** true if any processed cell carries a starter-only tag (may be used only as a start cell). */
	bool bFoundStarterOnlyTagged = false;

	/** true if any processed cell carries a finisher tag. */
	bool bFoundFinisherTagged = false;

	/** true if any processed cell carries a finisher-only tag (may be used only as an end cell). */
	bool bFoundFinisherOnlyTagged = false;

	/**
	 * true if any cell in the pool declares required context tags. When false, the context-tag gate in
	 * FilterCellInputData can be skipped for every candidate because no cell could ever be gated by it.
	 * @remark Populated by the production constructor; the test-only constructor leaves this false, so a test
	 *         exercising the context-tag gate through FilterCellInputData must set it explicitly.
	 */
	bool bAnyContextTagsRequired = false;

	/**
	 * true if any cell in the pool declares tag-counter constraints. When false, the tag-counter gate in
	 * FilterCellInputData can be skipped for every candidate because no cell could ever be gated by it.
	 * @remark Populated by the production constructor; the test-only constructor leaves this false, so a test
	 *         exercising the tag-counter gate through FilterCellInputData must set it explicitly.
	 */
	bool bAnyTagCounterConstraints = false;
};

/**
 * Represents input configuration data for a procedural generation cell.
 * 
 * This structure holds all the parameters and constraints that define how a cell
 * can be used during procedural generation, including placement rules, usage limits,
 * and junction connection points.
 */
struct NEXUSWORLDASSEMBLY_API FNVirtualCellData
{
	/** Assembly tags describing this cell's behavior, copied through to generated cells. */
	FGameplayTagContainer AssemblyTags;

	/** Context tags that must be present in the phases ContextTags context for this cell to be eligible for selection. */
	FGameplayTagContainer ContextTagsRequired;

	/** Context tags this cell contributes/adds to the overall assembly operation / phase ContextTags once it is placed. */
	FGameplayTagContainer ContextTagsAdded;
	
	TArray<FNGameplayTagCounterConstraint> TagCounterConstraints;
	
	TArray<FNGameplayTagCounterOperation> TagCounterOperations;

	/**
	 * A minimum number of times this cell must be used in the generated FNAssemblyGraph.
	 * @note A value of 0 indicates no minimum constraint.
	 * @remark Enforced in FNVirtualOrganContext::CheckGraph against UsedCount, skipping combined Unique + RequiredAny
	 *         cells and cells whose MinimumCount exceeds a positive MaximumCount (an unsatisfiable configuration).
	 */
	int32 MinimumCount = 0;

	/**
	 * The maximum number of times this cell can be used in the generated FNAssemblyGraph.
	 * @note A value of 0 indicates no maximum constraint (unlimited usage).
	 */
	int32 MaximumCount = 0;

	/**
	 * The minimum number of cell links away this cell must be to be used again.
	 * @note A value of 0 indicates no constraint.
	 */
	int32 MinimumNodeDistance = 1;
	
	/**
	 * The minimum graph depth at which this cell may be used, as a 1-based NodeDepth (the start cell is depth 1,
	 * its direct neighbours depth 2, and so on). A cell with MinimumNodeDepth = N first becomes eligible at
	 * NodeDepth N.
	 * @note A value of 0 indicates no constraint (a value of 1 is the start cell and is likewise unconstrained).
	 * @remark Gating is enforced in FNVirtualOrganContext::FilterCellInputData against the candidate's prospective
	 *         NodeDepth (the source node's NodeDepth + 1). See NMinimumNodeDepthTests.cpp before changing it.
	 */
	int32 MinimumNodeDepth = 0;

	/**
	 * The maximum graph depth at which this cell may still be used, as a 1-based NodeDepth (the start cell is
	 * depth 1, its direct neighbours depth 2, and so on). A cell with MaximumNodeDepth = N is last eligible at
	 * NodeDepth N; MaximumNodeDepth = 1 restricts the cell to the start cell only.
	 * @note A value of 0 indicates no constraint.
	 * @remark Gating is enforced in FNVirtualOrganContext::FilterCellInputData against the candidate's prospective
	 *         NodeDepth (the source node's NodeDepth + 1). See NMaximumNodeDepthTests.cpp before changing it.
	 */
	int32 MaximumNodeDepth = 0;

	/** When true, this cell may only be placed toward DirectionConstraint relative to the organ's start point. */
	bool bHasDirectionConstraint = false;

	/**
	 * Compass heading, measured from the organ's start point out to the candidate's attach point, this cell is
	 * restricted to while bHasDirectionConstraint is set.
	 * @remark Gating is enforced in FNVirtualOrganContext::FilterCellInputData, which admits the candidate only
	 *         when its bearing lands within AssemblyDirectionTolerance degrees (+/-) of this heading. See
	 *         NFilterCellInputDataTests.cpp.
	 */
	ENCardinalDirection DirectionConstraint = ENCardinalDirection::North;
	
	/** 
	 * Relative weight for random selection during generation.
	 * @note Higher values increase the probability of this cell being chosen.
	 */
	int32 Weighting = 1;

	/** Tracks how many times this cell has been used during the current generation pass. */
	int32 UsedCount = 0;

	/** Root-level details and properties of this cell. */
	FNCellRootDetails CellDetails;

	/** 
	 * Map of junction connection points on this cell.
	 * Key: Junction identifier (int32)
	 * Value: Details about the junction including position, rotation, and socket size.
	 */
	TMap<int32, FNCellJunctionDetails> Junctions;

	/** 
	 * Reference to the cell template asset.
	 * @warning GameThread access ONLY!
	 * @remark This is used later when assembling on the game-thread and is copied to output.
	 */
	TObjectPtr<UNCell> Template;

	/**
	 * Is a MinimumCount set?
	 * @return True if a minimum usage count constraint is defined (MinimumCount > 0).
	 */
	bool HasMinimumCount() const { return MinimumCount > 0; }

	/**
	 * Is a MaximumCount set?
	 * @return True if a maximum usage count constraint is defined (MaximumCount > 0); 0 means unlimited.
	 */
	bool HasMaximumCount() const { return MaximumCount > 0; }

	/**
	 * Is the UNCell meant to be uniquely placed?
	 * @return True if this cell must be used exactly once (MinimumCount == 1 && MaximumCount == 1).
	 */
	bool IsUnique() const { return MinimumCount == 1 && MaximumCount == 1; }

	/**
	 * Checks if this cell can still be selected for placement based on usage constraints.
	 * @return True if the cell hasn't exceeded its maximum count or unique constraint.
	 */
	bool IsValidSelection() const
	{
		if (Junctions.IsEmpty()) return false;
		if (HasMaximumCount() && UsedCount >= MaximumCount) return false; // MaximumCount == 0 ⇒ unlimited
		if (IsUnique() && UsedCount > 0) return false;

		return true;
	}

	/**
	 * Checks if this cell can be selected for a connection with a specific socket size.
	 * @param SocketSize The required socket size for the connection.
	 * @return True if valid for selection AND has at least one junction matching the socket size.
	 */
	bool IsValidSelection(const FIntVector2 SocketSize) const
	{
		if (!IsValidSelection()) return false;
		return HasJunctions(SocketSize);
	}

	/**
	 * Does the cell contain any junction of the provided size?
	 * @param SocketSize The socket size to filter by.
	 * @return true/false if a junction was found
	 */
	bool HasJunctions(const FIntVector2 SocketSize) const
	{
		for (auto& Junction : Junctions)
		{
			if (Junction.Value.SocketSize == SocketSize)
			{
				return true;
			}
		}
		return false;
	}
	
	// TODO: Implement Required/etc to junctions here and in the cell node version
	
	/**
	 * Gets all junction keys that match a specific socket size.
	 * @param SocketSize The socket size to filter by.
	 * @return Array of junction keys with matching socket sizes.
	 */
	TArray<int32> GetJunctionKeys(const FIntVector2 SocketSize) const
	{
		TArray<int32> Keys;
		for (auto& Junction : Junctions)
		{
			if (Junction.Value.SocketSize == SocketSize)
			{
				Keys.Add(Junction.Key);
			}
		}
		return Keys;
	}

	/**
	 * Gets all junction keys for this cell.
	 * @return Array containing all junction identifiers.
	 */
	TArray<int32> GetJunctionKeys() const
	{
		TArray<int32> Keys;
		Junctions.GetKeys(Keys);
		return MoveTemp(Keys);
	}
};