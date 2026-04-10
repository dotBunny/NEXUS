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
	
	// Maybe this gets made a separate context (output context) so we can singular input context
	TArray<FNCellOutputData> CellOutputData;
	
	
	FNOrganGeneratorTaskContext(const FNProcGenOperationOrganContext* GeneratorContextMap, uint64 TaskSeed);
	
	
	bool IsValid() const { return bIsValid; };
	bool IsSuccessful() const { return bSuccessful; };
	
	
	FNWeightedIntegerArray GenerateWeightedCellInputIndices();
	
	// TODO: Can we do something to pre-understand the socket sizes of all the junctions on a specific thing? 
	// TODO: Need to make this a working set we remove from cause of unique
	FNWeightedIntegerArray GenerateWeightedStartCellIndices(FIntVector2 RequestedSocketSize);
	FNWeightedIntegerArray GenerateWeightedCellInputIndices(FIntVector2 RequestedSocketSize);

private:
	
	bool bIsValid = false;
	bool bSuccessful = false;
	uint64 Seed;
};
