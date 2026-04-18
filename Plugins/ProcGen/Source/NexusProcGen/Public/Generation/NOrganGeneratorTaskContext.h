// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NBoneInputData.h"
#include "NProcGenGraph.h"
#include "NCellInputData.h"
#include "NOrganGeneratorTaskAnalytics.h"
#include "Collections/NWeightedIntegerArray.h"

struct FNProcGenOperationOrganContext;
class UNOrganComponent;

struct FNCellInputDataFilter
{
	bool bRequireStart = false;
	bool bRequireEnd = false;
	
	FIntVector2 SocketSize = FIntVector2(0, 0);
	FQuat SourceQuat = FQuat();
};

class FNOrganGeneratorTaskContext
{
	//friend struct FNOrganGeneratorTask;
	friend struct FNOrganGeneratorBuildGraphTask;

public:
	
	FNOrganGeneratorTaskAnalytics Analytics;
	
	int32 MinimumCellCount = -1;
	int32 MaximumCellCount = -1;
	int32 MaximumRetryCount = 0;
	bool bUnbounded = false;
	
	FBoxSphereBounds Bounds;
	FVector Origin;
	TArray<FNBoneInputData> BoneInputData;
	TArray<FNCellInputData> CellInputData;
	
	
	TUniquePtr<FNProcGenGraph> CellGraph = nullptr;
	FNOrganGeneratorTaskContext(const FNProcGenOperationOrganContext* GeneratorContextMap, uint64 TaskSeed, FString TaskName);
	~FNOrganGeneratorTaskContext();
	
	uint64 GetSeed() const { return Seed; };
	bool IsSuccessful() const { return bSuccessful; };
	bool IsValid() const { return bIsValid; };
	
	bool CheckGraph() const;
	void FilterCellInputData(const FNCellInputDataFilter& Filter, FNWeightedIntegerArray& CellIndices, TMap<int32, TArray<int32>>& JunctionIndices);
	bool ResetForRetry();
	bool ValidateGraph();
	
private:

	int32 RetryCount = 0;
	bool bIsValid = false;
	bool bSuccessful = false;
	uint64 Seed;
	FString Name;
};
