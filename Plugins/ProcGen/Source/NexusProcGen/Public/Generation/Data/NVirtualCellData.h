// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Cell/NCell.h"
#include "Cell/NCellJunctionDetails.h"
#include "Cell/NCellRootDetails.h"

/**
 * Represents input configuration data for a procedural generation cell.
 * 
 * This structure holds all the parameters and constraints that define how a cell
 * can be used during procedural generation, including placement rules, usage limits,
 * and junction connection points.
 */
struct NEXUSPROCGEN_API FNVirtualCellData
{
	/** Whether the NCellLevelInstance should be spawned always relevant for networking purposes. */
	bool bAlwaysRelevant = false;
	
	/** Whether this cell can be used as a starting node in the generated FNProcGenGraph. */
	bool bCanBeStartNode = true;

	/** Whether this cell can be used as an ending/terminal node in the generated FNProcGenGraph. */
	bool bCanBeEndNode = true;

	/** 
	 * A minimum number of times this cell must be used in the generated FNProcGenGraph.
	 * @note A value of -1 indicates no minimum constraint.
	 */
	int32 MinimumCount = -1;

	/** 
	 * The maximum number of times this cell can be used in the generated FNProcGenGraph.
	 * @note A value of -1 indicates no maximum constraint.
	 */
	int32 MaximumCount = -1;

	/**
	 * The minimum number of cell links away this cell must be to be used again.
	 * @note A value of 0 indicates no constraint.
	 */
	int32 MinimumNodeDistance = 1;

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
	 * @return True if a minimum usage count constraint is defined. 
	 */
	bool HasMinimumCount() const { return MinimumCount > -1; }

	/** 
	 * Is a MaximumCount set?
	 * @return True if a maximum usage count constraint is defined. 
	 */
	bool HasMaximumCount() const { return MaximumCount > -1; }

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
		if (Junctions.Num() == 0 || MaximumCount == 0) return false;
		if (HasMaximumCount() && UsedCount == MaximumCount) return false;
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