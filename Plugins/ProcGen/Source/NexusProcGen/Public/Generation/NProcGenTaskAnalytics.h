// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Types/NIterationCounter.h"

struct FNProcGenTaskTimer
{
	double StartTime;
	double EndTime;
	double Duration;
	
	void Start()
	{
		StartTime = FPlatformTime::Seconds();
	}
	void Stop()
	{
		EndTime = FPlatformTime::Seconds();
		Duration = (EndTime -StartTime) * 1000.f;
	}
};
struct FNOrganGraphBuilderAnalytics
{
	FString Name;
	FNProcGenTaskTimer Timer = FNProcGenTaskTimer();
	
	int Iterations = 1;
	
	FNIterationCounter AddNullNodes;
	FNIterationCounter AddCellNodes;
	
	FNIterationCounter DiscardOutOfBoundsStart;
	FNIterationCounter DiscardWorldCollidingStart;
	
	FNIterationCounter DiscardOutOfBoundsCellNode;
	FNIterationCounter DiscardIntersectingCellNode;
	FNIterationCounter DiscardWorldCollidingCellNode;
	FNIterationCounter DiscardExistingNodeWorldCollidingCellNode;
	
	void NextIteration()
	{
		Iterations++;
		AddNullNodes.NextIteration();
		AddCellNodes.NextIteration();
		
		DiscardOutOfBoundsStart.NextIteration();
		DiscardWorldCollidingStart.NextIteration();
		
		DiscardOutOfBoundsCellNode.NextIteration();
		DiscardIntersectingCellNode.NextIteration();
		DiscardWorldCollidingCellNode.NextIteration();
		DiscardExistingNodeWorldCollidingCellNode.NextIteration();
	}
};
struct FNCollectGenerationPassesAnalytics
{
	int Phase = 0;
	FNProcGenTaskTimer Timer = FNProcGenTaskTimer();
};

struct FNSpawnCellProxiesAnalytics
{
	TArray<FName> Spawned;
	FNProcGenTaskTimer Timer = FNProcGenTaskTimer();
};

class FNProcGenTaskAnalytics
{
public:
	explicit FNProcGenTaskAnalytics(const FText& DisplayName) : DisplayName(DisplayName) {};
	
	void TaskGraphCreationStart();
	void TaskGraphCreationFinish();
	
	void CreateWorldContextStart();
	void CreateWorldContextFinish();
	
	void ProcessWorldContextStart();
	void ProcessWorldContextFinish();
	
	int OrganGraphBuilderCreate();
	void OrganGraphBuilderStart(int32 Index);
	void OrganGraphBuilderFinish(int32 Index);
	
	void OrganGraphBuilder_Init(int32 Index, const FString& Name, int32 MinimumCellCount, int32 MaximumCellCount, int32 MaximumRetryCount);
	
	void OrganGraphBuilder_AddNullNode(int32 Index);
	void OrganGraphBuilder_AddCellNode(int32 Index);
	
	void OrganGraphBuilder_DiscardOutOfBoundsStart(int32 Index);
	void OrganGraphBuilder_DiscardWorldCollidingStart(int32 Index);
	
	void OrganGraphBuilder_DiscardOutOfBoundsCellNode(int32 Index);
	void OrganGraphBuilder_DiscardIntersectingCellNode(int32 Index);
	void OrganGraphBuilder_DiscardWorldCollidingCellNode(int32 Index);
	void OrganGraphBuilder_DiscardExistingNodeWorldCollidingCellNode(int32 Index);
	
	void OrganGraphBuilder_NextIteration(int32 Index);
	
	int CollectGenerationPassesCreate();
	void CollectGenerationPassesStart(int32 Index);
	void CollectGenerationPassesFinish(int32 Index);
	
	void CreateSpawnCellsContextStart();
	void CreateSpawnCellsContextFinish();
	
	int SpawnCellProxiesCreate();
	void SpawnCellProxiesStart(int32 Index);
	void SpawnCellProxiesSpawned(int32 Index, FName Template);
	void SpawnCellProxiesFinish(int32 Index);
	
	void ProcGenFinalizeStart();
	void ProcGenFinalizeFinish();
	
	FString GetTimespanReport();
	FString GetCountersReport();
	
private:
	FText DisplayName;

#if !UE_BUILD_SHIPPING	
	FNProcGenTaskTimer TaskGraphCreationTimer = FNProcGenTaskTimer();
	FNProcGenTaskTimer CreateWorldContextTimer = FNProcGenTaskTimer();
	FNProcGenTaskTimer ProcessWorldContextTimer = FNProcGenTaskTimer();
	
	FNProcGenTaskTimer CreateSpawnCellsContextTimer = FNProcGenTaskTimer();
	FNProcGenTaskTimer ProcGenFinalizeTimer = FNProcGenTaskTimer();
	
	TArray<FNOrganGraphBuilderAnalytics> OrganGraphBuilderAnalytics;
	TArray<FNCollectGenerationPassesAnalytics> CollectGenerationPassesAnalytics;
	TArray<FNSpawnCellProxiesAnalytics> SpawnCellProxiesAnalytics;
#endif // !UE_BUILD_SHIPPING
};