// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NBoneInputData.h"
#include "NProcGenGraph.h"
#include "NCellInputData.h"
#include "Collections/NWeightedIntegerArray.h"

struct FNProcGenOperationOrganContext;
class UNOrganComponent;

struct FNCellInputDataFilter
{
	bool bRequireStart;
	bool bRequireEnd;
	
	FIntVector2 SocketSize;
	FQuat SourceQuat;
};

class FNOrganGeneratorTaskContext
{
	friend struct FNOrganGeneratorTask;
	friend struct FNOrganGeneratorBuildGraphTask;

public:
	
	int32 MinimumCellCount = -1;
	int32 MaximumCellCount = -1;
	bool bUnbounded = false;
	
	FBoxSphereBounds Bounds;
	FVector Origin;
	TArray<FNBoneInputData> BoneInputData;
	TArray<FNCellInputData> CellInputData;
	
	TUniquePtr<FNProcGenGraph> CellGraph = nullptr;
	
	FNOrganGeneratorTaskContext(const FNProcGenOperationOrganContext* GeneratorContextMap, uint64 TaskSeed);
	~FNOrganGeneratorTaskContext();
	
	bool IsValid() const { return bIsValid; };
	bool IsSuccessful() const { return bSuccessful; };
	
	void FilterCellInputData(const FNCellInputDataFilter& Filter, FNWeightedIntegerArray& CellIndices, TMap<int32, TArray<int32>>& JunctionIndices);

private:
	
	bool bIsValid = false;
	bool bSuccessful = false;
	uint64 Seed;
};
