// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/Contexts/NProcGenTaskGraphContext.h"

FNProcGenTaskGraphContext::FNProcGenTaskGraphContext(UWorld* OutputWorld, const uint32& OperationTicket, const FNProcGenOperationSettings& Settings)
{
	this->OperationTicket = OperationTicket;
	TargetWorld = OutputWorld;
	OperationSettings = Settings;
	
	// TBD
	
	// What is useful cross pass for generation? 
	// - Key location?
	
}
