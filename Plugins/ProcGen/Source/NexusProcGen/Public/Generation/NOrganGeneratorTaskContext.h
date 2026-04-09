// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NBoneInputData.h"
#include "NCellInputData.h"
#include "NCellOutputData.h"
#include "Collections/NWeightedIntegerArray.h"

struct FNProcGenOperationOrganContext;
class UNOrganComponent;

class FNOrganGeneratorTaskContext
{
	friend struct FNOrganGeneratorTask;
	friend struct FNOrganGeneratorBuildGraphTask;

public:
	
	int32 MinimumCellCount = -1;
	int32 MaximumCellCount = -1;
	
	FBoxSphereBounds Bounds;
	FVector Origin;
	TArray<FNBoneInputData> BoneInputData;
	TArray<FNCellInputData> CellInputData;
	
	// Maybe this gets made a seperate context (output context) so we can singular input context
	TArray<FNCellOutputData> CellOutputData;
	
	bool bSuccessful = false;
	
	FNOrganGeneratorTaskContext(const FNProcGenOperationOrganContext* GeneratorContextMap, uint64 TaskSeed);
	FNWeightedIntegerArray GenerateWeightedCellInputIndices();


private:
	
	uint64 Seed;
};
