// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Analytics/NOrganGraphBuilderAnalytics.h"
#include "Analytics/NProcessPassAnalytics.h"
#include "Analytics/NProcGenTaskTimer.h"
#include "Analytics/NSpawnCellProxiesAnalytics.h"

#if !UE_BUILD_SHIPPING	

#define N_PROC_GEN_ANALYTICS_CREATE AnalyticsPtr = MakeShared<FNProcGenTaskAnalytics, ESPMode::ThreadSafe>(Operation->GetDisplayName());
#define N_PROC_GEN_ANALYTICS_CONSTRUCTOR , const TSharedPtr<FNProcGenTaskAnalytics>& AnalyticsPtr
#define N_PROC_GEN_ANALYTICS_INITIALIZER , AnalyticsPtr(AnalyticsPtr.ToSharedRef())
#define N_PROC_GEN_ANALYTICS_CLASS_REF , AnalyticsPtr
#define N_PROC_GEN_ANALYTICS_SHARED_PTR	TSharedPtr<FNProcGenTaskAnalytics> AnalyticsPtr;
#define N_PROC_GEN_ANALYTICS_SHARED_REF TSharedRef<FNProcGenTaskAnalytics> AnalyticsPtr;
#define N_PROC_GEN_ANALYTICS_DELETE AnalyticsPtr.Reset();
#define N_PROC_GEN_ANALYTICS_INDEX_LOCAL int32 AnalyticsIndex = -1;

#define N_PROC_GEN_ANALYTICS(Method) \
	AnalyticsPtr->Method();
#define N_PROC_GEN_ANALYTICS_ONE_PARAM(Method, Parameter) \
	AnalyticsPtr->Method(Parameter);
#define N_PROC_GEN_ANALYTICS_TWO_PARAM(Method, Parameter1, Parameter2) \
	AnalyticsPtr->Method(Parameter1, Parameter2);
#define N_PROC_GEN_ANALYTICS_THREE_PARAM(Method, Parameter1, Parameter2, Parameter3) \
	AnalyticsPtr->Method(Parameter1, Parameter2, Parameter3);
#define N_PROC_GEN_ANALYTICS_FOUR_PARAM(Method, Parameter1, Parameter2, Parameter3, Parameter4) \
	AnalyticsPtr->Method(Parameter1, Parameter2, Parameter3, Parameter4);
#define N_PROC_GEN_ANALYTICS_FIVE_PARAM(Method, Parameter1, Parameter2, Parameter3, Parameter4, Parameter5) \
	AnalyticsPtr->Method(Parameter1, Parameter2, Parameter3, Parameter4, Parameter5);
#define N_PROC_GEN_ANALYTICS_INDEX(Method) \
	AnalyticsPtr->Method(AnalyticsIndex);
#define N_PROC_GEN_ANALYTICS_INDEX_SET(Method) \
	AnalyticsIndex = AnalyticsPtr->Method();
#define N_PROC_GEN_ANALYTICS_INDEX_DEFINE(Method) \
	const int AnalyticsIndex = AnalyticsPtr->Method();
#else
#define N_PROC_GEN_ANALYTICS_CREATE
#define N_PROC_GEN_ANALYTICS_CONSTRUCTOR
#define N_PROC_GEN_ANALYTICS_INITIALIZER
#define N_PROC_GEN_ANALYTICS_CLASS_REF
#define N_PROC_GEN_ANALYTICS_SHARED_PTR
#define N_PROC_GEN_ANALYTICS_SHARED_REF
#define N_PROC_GEN_ANALYTICS_DELETE
#define N_PROC_GEN_ANALYTICS_INDEX_LOCAL
#define N_PROC_GEN_ANALYTICS(Method)
#define N_PROC_GEN_ANALYTICS_ONE_PARAM(Method, Parameter)
#define N_PROC_GEN_ANALYTICS_TWO_PARAM(Method, Parameter1, Parameter2)
#define N_PROC_GEN_ANALYTICS_THREE_PARAM(Method, Parameter1, Parameter2, Parameter3)
#define N_PROC_GEN_ANALYTICS_FOUR_PARAM(Method, Parameter1, Parameter2, Parameter3, Parameter4)
#define N_PROC_GEN_ANALYTICS_FIVE_PARAM(Method, Parameter1, Parameter2, Parameter3, Parameter4, Parameter5)
#define N_PROC_GEN_ANALYTICS_INDEX(Method)
#define N_PROC_GEN_ANALYTICS_INDEX_SET(Method)
#define N_PROC_GEN_ANALYTICS_INDEX_DEFINE(Method)
#endif // !UE_BUILD_SHIPPING

class FNProcGenTaskAnalytics
{
public:
	explicit FNProcGenTaskAnalytics(const FText& DisplayName) : DisplayName(DisplayName) {};
	
	void TaskGraphCreationStart();
	void TaskGraphCreationFinish();
	
	void CreateVirtualWorldContextStart();
	void CreateVirtualWorldContextFinish();
	
	void ProcessVirtualWorldContextStart();
	void ProcessVirtualWorldContextFinish();
	
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
	
	FNProcGenTaskTimer TaskGraphCreationTimer = FNProcGenTaskTimer();
	FNProcGenTaskTimer CreateVirtualWorldContextTimer = FNProcGenTaskTimer();
	FNProcGenTaskTimer ProcessVirtualWorldContextTimer = FNProcGenTaskTimer();
	
	FNProcGenTaskTimer CreateSpawnCellsContextTimer = FNProcGenTaskTimer();
	FNProcGenTaskTimer ProcGenFinalizeTimer = FNProcGenTaskTimer();
	
	TArray<FNOrganGraphBuilderAnalytics> OrganGraphBuilderAnalytics;
	TArray<FNProcessPassAnalytics> ProcessPassAnalytics;
	TArray<FNSpawnCellProxiesAnalytics> SpawnCellProxiesAnalytics;
};