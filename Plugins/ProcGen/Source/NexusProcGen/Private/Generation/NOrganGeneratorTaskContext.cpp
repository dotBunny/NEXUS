// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NOrganGeneratorTaskContext.h"

#include "Generation/NProcGenOperationContext.h"
#include "Organ/NOrganComponent.h"

FNOrganGeneratorTaskContext::FNOrganGeneratorTaskContext(FNOrganGenerationContext* GeneratorContextMap, const uint64 TaskSeed)
	: Seed(TaskSeed)
{
	// Collect all thread safe data about organ
	// All possible cells that could go in organ? 
	
	// - build out all available tissue set information
	
	// - size of the organ / world space / etc
	// - volume size
}
