// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Generation/Graph/NProcGenGraphCellNode.h"

class FNSpawnCellsContext
{
public:
	int CurrentIndex = 0;
	TArray<FNProcGenGraphCellNode*> Nodes;
	UWorld* World;
	uint32 OperationTicket;
	bool bPreloadLevel;
	bool bSpawnLevelInstance;
	
	explicit FNSpawnCellsContext(UWorld* TargetWorld, const uint32 OperationTicket, const bool bPreloadLevel, const bool bSpawnLevelInstance)
		: World(TargetWorld), OperationTicket(OperationTicket), bPreloadLevel(bPreloadLevel), bSpawnLevelInstance(bSpawnLevelInstance)
	{
	}
};
