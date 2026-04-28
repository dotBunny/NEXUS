// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenTaskAnalytics.h"

namespace NEXUS::ProcGen::Analytics
{
	inline float DurationMeasurement = 1000.f;
}

#if !UE_BUILD_SHIPPING	
void FNProcGenTaskAnalytics::TaskGraphCreationStart()
{
	TaskGraphCreationStartTime = FPlatformTime::Seconds();
};
void FNProcGenTaskAnalytics::TaskGraphCreationFinish()
{
	TaskGraphCreationEndTime = FPlatformTime::Seconds();
	TaskGraphCreationDuration = (TaskGraphCreationEndTime - TaskGraphCreationStartTime) * NEXUS::ProcGen::Analytics::DurationMeasurement;
}
void FNProcGenTaskAnalytics::CreateWorldContextStart()
{
	CreateWorldContextStartTime = FPlatformTime::Seconds();
}
void FNProcGenTaskAnalytics::CreateWorldContextFinish()
{
	CreateWorldContextEndTime = FPlatformTime::Seconds();
	CreateWorldContextDuration = (CreateWorldContextEndTime - CreateWorldContextStartTime) * NEXUS::ProcGen::Analytics::DurationMeasurement;
}
void FNProcGenTaskAnalytics::ProcessWorldContextStart()
{
	ProcessWorldContextStartTime = FPlatformTime::Seconds();
}

void FNProcGenTaskAnalytics::ProcessWorldContextFinish()
{
	ProcessWorldContextEndTime = FPlatformTime::Seconds();
	ProcessWorldContextDuration = (ProcessWorldContextEndTime - ProcessWorldContextStartTime) * NEXUS::ProcGen::Analytics::DurationMeasurement;
}

int FNProcGenTaskAnalytics::OrganGraphBuilderCreate()
{
	OrganGraphBuilderAnalytics.Add(FNOrganGraphBuilderAnalytics());
	return OrganGraphBuilderAnalytics.Num() - 1;
}
void FNProcGenTaskAnalytics::OrganGraphBuilderStart(int32 Index)
{
	FNOrganGraphBuilderAnalytics& OrganAnalytics = OrganGraphBuilderAnalytics[Index];
	OrganAnalytics.StartTime = FPlatformTime::Seconds();
}
void FNProcGenTaskAnalytics::OrganGraphBuilder_Init(int32 Index, const FString& Name, int32 MinimumCellCount, int32 MaximumCellCount, int32 MaximumRetryCount)
{
	FNOrganGraphBuilderAnalytics& OrganAnalytics = OrganGraphBuilderAnalytics[Index];
	OrganAnalytics.Name = Name;
}
void FNProcGenTaskAnalytics::OrganGraphBuilder_AddNullNode(int32 Index)
{
	FNOrganGraphBuilderAnalytics& OrganAnalytics = OrganGraphBuilderAnalytics[Index];
	OrganAnalytics.AddNullNodes.Increment();
}
void FNProcGenTaskAnalytics::OrganGraphBuilder_AddCellNode(int32 Index)
{
	FNOrganGraphBuilderAnalytics& OrganAnalytics = OrganGraphBuilderAnalytics[Index];
	OrganAnalytics.AddCellNodes.Increment();
}
void FNProcGenTaskAnalytics::OrganGraphBuilder_DiscardOutOfBoundsStart(int32 Index)
{
	FNOrganGraphBuilderAnalytics& OrganAnalytics = OrganGraphBuilderAnalytics[Index];
	OrganAnalytics.DiscardOutOfBoundsStart.Increment();
}
void FNProcGenTaskAnalytics::OrganGraphBuilder_DiscardWorldCollidingStart(int32 Index)
{
	FNOrganGraphBuilderAnalytics& OrganAnalytics = OrganGraphBuilderAnalytics[Index];
	OrganAnalytics.DiscardWorldCollidingStart.Increment();
}
void FNProcGenTaskAnalytics::OrganGraphBuilder_DiscardOutOfBoundsCellNode(int32 Index)
{
	FNOrganGraphBuilderAnalytics& OrganAnalytics = OrganGraphBuilderAnalytics[Index];
	OrganAnalytics.DiscardOutOfBoundsCellNode.Increment();
}
void FNProcGenTaskAnalytics::OrganGraphBuilder_DiscardIntersectingCellNode(int32 Index)
{
	FNOrganGraphBuilderAnalytics& OrganAnalytics = OrganGraphBuilderAnalytics[Index];
	OrganAnalytics.DiscardIntersectingCellNode.Increment();
}
void FNProcGenTaskAnalytics::OrganGraphBuilder_DiscardWorldCollidingCellNode(int32 Index)
{
	FNOrganGraphBuilderAnalytics& OrganAnalytics = OrganGraphBuilderAnalytics[Index];
	OrganAnalytics.DiscardWorldCollidingCellNode.Increment();
}

void FNProcGenTaskAnalytics::OrganGraphBuilder_NextIteration(int32 Index)
{
	FNOrganGraphBuilderAnalytics& OrganAnalytics = OrganGraphBuilderAnalytics[Index];
	OrganAnalytics.NextIteration();
}

void FNProcGenTaskAnalytics::OrganGraphBuilderFinish(int32 Index)
{
	FNOrganGraphBuilderAnalytics& OrganAnalytics = OrganGraphBuilderAnalytics[Index];
	OrganAnalytics.EndTime = FPlatformTime::Seconds();
	OrganAnalytics.Duration = (OrganAnalytics.EndTime - OrganAnalytics.StartTime) * NEXUS::ProcGen::Analytics::DurationMeasurement;
}
#else
void FNProcGenTaskAnalytics::TaskGraphCreationStart() {}
void FNProcGenTaskAnalytics::TaskGraphCreationFinish() {}

void FNProcGenTaskAnalytics::CreateWorldContextStart() {}
void FNProcGenTaskAnalytics::CreateWorldContextFinish() {}

void FNProcGenTaskAnalytics::ProcessWorldContextStart() {}
void FNProcGenTaskAnalytics::ProcessWorldContextFinish() {}

int FNProcGenTaskAnalytics::OrganGraphBuilderCreate() { return -1; }
void FNProcGenTaskAnalytics::OrganGraphBuilderStart(int32 Index) {}
void FNProcGenTaskAnalytics::OrganGraphBuilderFinish(int32 Index) {}
void FNProcGenTaskAnalytics::OrganGraphBuilder_Init(int32 Index, const FString& Name, 
	int32 MinimumCellCount, int32 MaximumCellCount, int32 MaximumRetryCount) {}
void FNProcGenTaskAnalytics::OrganGraphBuilder_AddNullNode(int32 Index) {}
void FNProcGenTaskAnalytics::OrganGraphBuilder_AddCellNode(int32 Index) {}
void FNProcGenTaskAnalytics::OrganGraphBuilder_DiscardOutOfBoundsStart(int32 Index) {}
void FNProcGenTaskAnalytics::OrganGraphBuilder_DiscardWorldCollidingStart(int32 Index) {}
void FNProcGenTaskAnalytics::OrganGraphBuilder_DiscardOutOfBoundsCellNode(int32 Index) {}
void FNProcGenTaskAnalytics::OrganGraphBuilder_DiscardIntersectingCellNode(int32 Index) {}
void FNProcGenTaskAnalytics::OrganGraphBuilder_DiscardWorldCollidingCellNode(int32 Index) {}
void FNProcGenTaskAnalytics::OrganGraphBuilder_NextIteration(int32 Index) {}
#endif // !UE_BUILD_SHIPPING	