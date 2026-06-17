// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Assembly/Graph/NAssemblyGraphCellNode.h"

/**
 * Shared context used by FNCreateSpawnsTask and FNSpawnCellProxiesTask to drive timesliced
 * cell-proxy spawning.
 *
 * Holds the target world plus the flat list of cell nodes that need proxies, along with a
 * cursor indicating how far the spawn pass has progressed. The level-instance flags forward
 * the operation's settings to each spawned proxy.
 */
class FNSpawnContext
{
public:
	/** World the proxies will be spawned into. */
	TObjectPtr<UWorld> World;

	/** Operation identifier propagated into spawned proxies for traceability. */
	int32 OperationTicket;

	/** When true, level instances should be preloaded as part of the spawn pass. */
	bool bPreloadLevels;

	/** When true, level instances should be created for each spawned proxy. */
	bool bSpawnLevelInstances;

	/** Set to request early termination of the spawn pass. */
	TAtomic<bool> bCancelled{false};

	/** Cursor into CellNodes tracking how many entries have already been spawned. */
	int32 CellNodesCreateCurrentIndex = 0;

	/**
	 * Status-channel id for the spawn pass, opened on the first dispatch and reused across timeslices so
	 * the progress bar keeps moving while spawning re-dispatches frame to frame. INDEX_NONE until opened.
	 */
	int32 SpawnStatusChannelId = INDEX_NONE;
	
	int32 CellNodesLoadCurrentIndex = 0;
	
	/** Per-frame time budget for spawning cell proxies, in seconds. */
	float CellTimeSlice = 0.002f;

	/** Flattened list of cell nodes awaiting proxy spawn, populated by FNCreateSpawnsTask. */
	TArray<FNAssemblyGraphCellNode*> CellNodes;

	explicit FNSpawnContext(UWorld* TargetWorld, const int32 OperationTicket, const bool bPreloadLevels, const bool bSpawnLevelInstances, const float CellSpawningTimeSlice = 0.002f)
		: World(TargetWorld), OperationTicket(OperationTicket), bPreloadLevels(bPreloadLevels), bSpawnLevelInstances(bSpawnLevelInstances), CellTimeSlice(CellSpawningTimeSlice) {}
};
