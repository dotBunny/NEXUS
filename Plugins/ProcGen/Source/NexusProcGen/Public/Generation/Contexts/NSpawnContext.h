// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Generation/Graph/NProcGenGraphCellNode.h"

class FNSpawnContext
{
public:

	UWorld* World;
	uint32 OperationTicket;
	
	bool bPreloadLevels;
	bool bSpawnLevelInstances;
	
	int CellNodesCurrentIndex = 0;
	TArray<FNProcGenGraphCellNode*> CellNodes;
	
	explicit FNSpawnContext(UWorld* TargetWorld, const uint32 OperationTicket, const bool bPreloadLevels, const bool bSpawnLevelInstances)
		: World(TargetWorld), OperationTicket(OperationTicket), bPreloadLevels(bPreloadLevels), bSpawnLevelInstances(bSpawnLevelInstances) {}
};
