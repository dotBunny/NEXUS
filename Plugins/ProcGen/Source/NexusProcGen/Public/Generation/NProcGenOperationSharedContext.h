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
	
	explicit FNProcGenOperationSharedContext(UWorld* OutputWorld, bool bCreateInstances= false, bool bLoadInstances = false);
};
