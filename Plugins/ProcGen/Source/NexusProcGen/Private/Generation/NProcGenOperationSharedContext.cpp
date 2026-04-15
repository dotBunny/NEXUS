// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenOperationSharedContext.h"

FNProcGenOperationSharedContext::FNProcGenOperationSharedContext(UWorld* OutputWorld, const bool bReplicateInstances, const bool bCreateInstances, const bool bLoadInstances)
{
	TargetWorld = OutputWorld;
	
	bCreateLevelInstances = bCreateInstances;
	bLoadLevelInstances = bLoadInstances;
	bReplicateLevelInstances = bReplicateInstances;
	
	// TBD
	
	// What is useful cross pass for generation? 
	// - Key location?
	
}
