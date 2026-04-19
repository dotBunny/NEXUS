// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NProcGenOperationSettings.h"
#include "Cell/NCellProxy.h"

class FNProcGenTaskGraphContext
{
public:
	/**
	 * The UWorld to target when creating or querying from the NProcGenTaskGraph.
	 */
	UWorld* TargetWorld;

	/**
	 * The proxies that were created from the evaluated task graph.
	 */
	TArray<ANCellProxy*> CreatedProxies;

	/**
	 * Operation-specific settings effecting the behavior of some bits of the graph.
	 */
	FNProcGenOperationSettings OperationSettings;
	
	explicit FNProcGenTaskGraphContext(UWorld* OutputWorld, const FNProcGenOperationSettings& Settings);
};
