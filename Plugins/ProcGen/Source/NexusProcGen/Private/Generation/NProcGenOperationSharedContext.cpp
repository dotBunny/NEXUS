// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenOperationSharedContext.h"

FNProcGenOperationSharedContext::FNProcGenOperationSharedContext(UWorld* OutputWorld, const FNProcGenOperationSettings& Settings)
{
	TargetWorld = OutputWorld;
	OperationSettings = Settings;
	
	// TBD
	
	// What is useful cross pass for generation? 
	// - Key location?
	
}
