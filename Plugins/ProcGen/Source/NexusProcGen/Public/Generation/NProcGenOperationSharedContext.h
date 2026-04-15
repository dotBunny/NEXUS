// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NProcGenOperationSettings.h"
#include "Cell/NCellProxy.h"

class FNProcGenOperationSharedContext
{
public:
	UWorld* TargetWorld;
	TArray<ANCellProxy*> CreatedProxies;
	FNProcGenOperationSettings OperationSettings;
	
	explicit FNProcGenOperationSharedContext(UWorld* OutputWorld, const FNProcGenOperationSettings& Settings);
};
