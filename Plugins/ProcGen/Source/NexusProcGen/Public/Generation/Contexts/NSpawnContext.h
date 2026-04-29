// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Generation/Graph/NProcGenGraphCellNode.h"

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
	UWorld* World;

	/** Operation identifier propagated into spawned proxies for traceability. */
	uint32 OperationTicket;

	/** When true, level instances should be preloaded as part of the spawn pass. */
	bool bPreloadLevels;

	/** When true, level instances should be created for each spawned proxy. */
	bool bSpawnLevelInstances;

	/** Cursor into CellNodes tracking how many entries have already been spawned. */
	int32 CellNodesCurrentIndex = 0;

	/** Flattened list of cell nodes awaiting proxy spawn, populated by FNCreateSpawnsTask. */
	TArray<FNProcGenGraphCellNode*> CellNodes;

	explicit FNSpawnContext(UWorld* TargetWorld, const uint32 OperationTicket, const bool bPreloadLevels, const bool bSpawnLevelInstances)
		: World(TargetWorld), OperationTicket(OperationTicket), bPreloadLevels(bPreloadLevels), bSpawnLevelInstances(bSpawnLevelInstances) {}
};
