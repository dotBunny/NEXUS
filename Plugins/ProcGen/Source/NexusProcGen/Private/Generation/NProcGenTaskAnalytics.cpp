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
	TaskGraphCreationTimer.Start();
};
void FNProcGenTaskAnalytics::TaskGraphCreationFinish()
{
	TaskGraphCreationTimer.Stop();
}
void FNProcGenTaskAnalytics::CreateWorldContextStart()
{
	CreateWorldContextTimer.Start();
}
void FNProcGenTaskAnalytics::CreateWorldContextFinish()
{
	CreateWorldContextTimer.Stop();
}
void FNProcGenTaskAnalytics::ProcessWorldContextStart()
{
	ProcessWorldContextTimer.Start();
}

void FNProcGenTaskAnalytics::ProcessWorldContextFinish()
{
	ProcessWorldContextTimer.Stop();
}

int FNProcGenTaskAnalytics::OrganGraphBuilderCreate()
{
	OrganGraphBuilderAnalytics.Add(FNOrganGraphBuilderAnalytics());
	return OrganGraphBuilderAnalytics.Num() - 1;
}
void FNProcGenTaskAnalytics::OrganGraphBuilderStart(int32 Index)
{
	FNOrganGraphBuilderAnalytics& OrganAnalytics = OrganGraphBuilderAnalytics[Index];
	OrganAnalytics.Timer.Start();
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

int FNProcGenTaskAnalytics::CollectGenerationPassesCreate()
{
	CollectGenerationPassesAnalytics.Add(FNCollectGenerationPassesAnalytics());
	return CollectGenerationPassesAnalytics.Num() - 1;
}

void FNProcGenTaskAnalytics::CollectGenerationPassesStart(int32 Index)
{
	FNCollectGenerationPassesAnalytics& CollectionAnalytics = CollectGenerationPassesAnalytics[Index];
	CollectionAnalytics.Timer.Start();
}

void FNProcGenTaskAnalytics::CollectGenerationPassesFinish(int32 Index)
{
	FNCollectGenerationPassesAnalytics& CollectionAnalytics = CollectGenerationPassesAnalytics[Index];
	CollectionAnalytics.Timer.Stop();
}

void FNProcGenTaskAnalytics::SpawnCellProxiesStart()
{
	SpawnCellProxiesTimer.Start();
}

void FNProcGenTaskAnalytics::SpawnCellProxiesFinish()
{
	SpawnCellProxiesTimer.Stop();
}

void FNProcGenTaskAnalytics::ProcGenFinalizeStart()
{
	ProcGenFinalizeTimer.Start();
}

void FNProcGenTaskAnalytics::ProcGenFinalizeFinish()
{
	ProcGenFinalizeTimer.Stop();
}

FString FNProcGenTaskAnalytics::ToString()
{
	// TODO: IMPLEMENT
	return TEXT("TO IMPLEMENT");
}

void FNProcGenTaskAnalytics::OrganGraphBuilderFinish(int32 Index)
{
	FNOrganGraphBuilderAnalytics& OrganAnalytics = OrganGraphBuilderAnalytics[Index];
	OrganAnalytics.Timer.Stop();
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
FString FNProcGenTaskAnalytics::ToString() { return TEXT("NProcGenTaskAnalytics are not available in UE_BUILD_SHIPPING."); }

int FNProcGenTaskAnalytics::CollectGenerationPassesCreate() { return -1; }
void FNProcGenTaskAnalytics::CollectGenerationPassesStart(int32 Index) {}
void FNProcGenTaskAnalytics::CollectGenerationPassesFinish(int32 Index) {}

void FNProcGenTaskAnalytics::SpawnCellProxiesStart() {}
void FNProcGenTaskAnalytics::SpawnCellProxiesFinish() {}

void FNProcGenTaskAnalytics::ProcGenFinalizeStart() {}
void FNProcGenTaskAnalytics::ProcGenFinalizeFinish() {}

#endif // !UE_BUILD_SHIPPING	