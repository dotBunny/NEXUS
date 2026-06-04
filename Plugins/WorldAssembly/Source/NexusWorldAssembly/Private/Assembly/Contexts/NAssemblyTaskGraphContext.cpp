// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"

FNAssemblyTaskGraphContext::FNAssemblyTaskGraphContext(UWorld* OutputWorld, const int32& OperationTicket, const FNAssemblyOperationSettings& Settings)
{
	this->OperationTicket = OperationTicket;
	TargetWorld = OutputWorld;
	OperationSettings = Settings;
	
	ContextTags = Settings.ContextTags;
	TagCounter = Settings.TagCounters;
}
