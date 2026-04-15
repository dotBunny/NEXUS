// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Cell/NCellProxy.h"

class FNProcGenOperationSharedContext
{
public:
	UWorld* TargetWorld;
	TArray<ANCellProxy*> CreatedProxies;
	bool bCreateLevelInstances = false;
	bool bLoadLevelInstances = false;
	bool bReplicateLevelInstances = true;
	
	explicit FNProcGenOperationSharedContext(UWorld* OutputWorld, bool bReplicateInstances = true, bool bCreateInstances = false,  bool bLoadInstances = false);
};
