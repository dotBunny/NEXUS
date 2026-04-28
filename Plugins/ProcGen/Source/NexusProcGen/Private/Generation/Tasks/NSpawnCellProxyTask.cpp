// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Tasks/NSpawnCellProxyTask.h"

void FNSpawnCellProxyTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
	for (const auto CellNode : Nodes)
	{
		// Spawn proxy instance
		ANCellProxy* Proxy = ANCellProxy::CreateInstance(World, OperationTicket, CellNode, bPreloadLevel);
		
		// Registered with global?
		TaskGraphContextPtr->CreatedProxies.Add(Proxy);
			
		// What about creating the instance?
		if  (bSpawnLevelInstance)
		{
			Proxy->CreateLevelInstance();
			Proxy->LoadLevelInstance();
		}
	}
}
