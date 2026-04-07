// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NBoneInputData.h"
#include "NCellInputData.h"
#include "NCellOutputData.h"

struct FNProcGenOperationOrganContext;
class UNOrganComponent;

class FNOrganGeneratorTaskContext
{
	friend struct FNOrganGeneratorTask;
	friend struct FNOrganGeneratorBuildGraphTask;

public:
	
	FBoxSphereBounds Bounds;
	FVector Origin;
	TArray<FNBoneInputData> BoneInputData;
	TArray<FNCellInputData> CellInputData;
	
	TArray<FNCellOutputData> CellOutputData;
	
	FNOrganGeneratorTaskContext(const FNProcGenOperationOrganContext* GeneratorContextMap, uint64 TaskSeed);
	
private:
	uint64 Seed;
};
