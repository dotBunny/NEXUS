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
	const int Phase = CollectGenerationPassesAnalytics.Num() - 1;
	CollectGenerationPassesAnalytics.Last().Phase = Phase;
	return Phase;
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

void FNProcGenTaskAnalytics::CreateSpawnCellsContextStart()
{
	CreateSpawnCellsContextTimer.Start();
}

void FNProcGenTaskAnalytics::CreateSpawnCellsContextFinish()
{
	CreateSpawnCellsContextTimer.Stop();
}

void FNProcGenTaskAnalytics::ProcGenFinalizeStart()
{
	ProcGenFinalizeTimer.Start();
}

void FNProcGenTaskAnalytics::ProcGenFinalizeFinish()
{
	ProcGenFinalizeTimer.Stop();
}

FString FNProcGenTaskAnalytics::GetTimespanReport()
{
	FStringBuilderBase Builder = FStringBuilderBase();
	
	Builder.Appendf(TEXT("[%s] Timespans\n"), *DisplayName.ToString());
	
	// Report Timespans
	double DurationTotal = 0;
	
	Builder.Appendf(TEXT("[G]\tTaskGraph Creation: %f ms\n"), TaskGraphCreationTimer.Duration);
	DurationTotal += TaskGraphCreationTimer.Duration;
	
	Builder.Appendf(TEXT("[G]\tCreate WorldContext: %f ms\n"), CreateWorldContextTimer.Duration);
	DurationTotal += CreateWorldContextTimer.Duration;
	
	Builder.Appendf(TEXT("[B]\tProcess WorldContext: %f ms\n"), ProcessWorldContextTimer.Duration);
	DurationTotal += ProcessWorldContextTimer.Duration;
	
	double OrganGraphBuilderDurationTotal = 0;
	Builder.Append(TEXT("\tOrganGraph Builders:\n"));
	for (const auto OrganGraphBuilderAnalytic : OrganGraphBuilderAnalytics)
	{
		Builder.Appendf(TEXT("[B]\t\t%s (%i): %f ms\n"), *OrganGraphBuilderAnalytic.Name, OrganGraphBuilderAnalytic.Iterations, OrganGraphBuilderAnalytic.Timer.Duration);
		DurationTotal += OrganGraphBuilderAnalytic.Timer.Duration;
		OrganGraphBuilderDurationTotal += OrganGraphBuilderAnalytic.Timer.Duration;
	}
	Builder.Appendf(TEXT("\t\tTotal=%f ms:\n"), OrganGraphBuilderDurationTotal);
	
	Builder.Append(TEXT("\tCollect Generation Passes:\n"));
	for (const auto CollectGenerationPassesAnalytic : CollectGenerationPassesAnalytics)
	{
		Builder.Appendf(TEXT("[B]\t\tPhase %i: %f ms\n"), CollectGenerationPassesAnalytic.Phase, CollectGenerationPassesAnalytic.Timer.Duration);
		DurationTotal += CollectGenerationPassesAnalytic.Timer.Duration;
	}
	
	Builder.Appendf(TEXT("[B]\tCreate SpawnCellsContext: %f\n"), CreateSpawnCellsContextTimer.Duration);
	DurationTotal += CreateSpawnCellsContextTimer.Duration;
	
	Builder.Appendf(TEXT("[G]\tProcGen Finalize: %f ms\n"), ProcGenFinalizeTimer.Duration);
	DurationTotal += ProcGenFinalizeTimer.Duration;
	Builder.Appendf(TEXT("\tTotal=%f ms\n"), DurationTotal);
	
	return Builder.ToString();
}

FString FNProcGenTaskAnalytics::GetCountersReport()
{
	FStringBuilderBase Builder = FStringBuilderBase();
	
	Builder.Appendf(TEXT("[%s] Counters\n"), *DisplayName.ToString());
	
	Builder.Append(TEXT("\tOrganGraph Builders:\n"));
	for (const auto OrganGraphBuilderAnalytic : OrganGraphBuilderAnalytics)
	{
		// Output data on each iteration
		for (int i = 0; i < OrganGraphBuilderAnalytic.Iterations; i++)
		{
			Builder.Appendf(TEXT("\t\t%s (%i)\n"), *OrganGraphBuilderAnalytic.Name, OrganGraphBuilderAnalytic.Iterations);
			Builder.Append(TEXT("\t\t\tFalse Starts:\n"));
			Builder.Appendf(TEXT("\t\t\t\tOut Of Bounds: %i\n"), OrganGraphBuilderAnalytic.DiscardOutOfBoundsStart.Counter[i]);
			Builder.Appendf(TEXT("\t\t\t\tWorld Colliding: %i\n"), OrganGraphBuilderAnalytic.DiscardWorldCollidingStart.Counter[i]);
			
			Builder.Append(TEXT("\t\t\tAdding Nodes:\n"));
			Builder.Appendf(TEXT("\t\t\t\tNull: %i\n"), OrganGraphBuilderAnalytic.AddNullNodes.Counter[i]);
			Builder.Appendf(TEXT("\t\t\t\tCell: %i\n"), OrganGraphBuilderAnalytic.AddCellNodes.Counter[i]);
			
			Builder.Append(TEXT("\t\t\tBad Placement (Cell):\n"));
			Builder.Appendf(TEXT("\t\t\t\tIntersecting: %i\n"), OrganGraphBuilderAnalytic.DiscardIntersectingCellNode.Counter[i]);
			Builder.Appendf(TEXT("\t\t\t\tWorld Colliding: %i\n"), OrganGraphBuilderAnalytic.DiscardWorldCollidingCellNode.Counter[i]);
			Builder.Appendf(TEXT("\t\t\t\tOut Of Bounds: %i\n"), OrganGraphBuilderAnalytic.DiscardOutOfBoundsCellNode.Counter[i]);
		}
	}
	
	return Builder.ToString();
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
FString FNProcGenTaskAnalytics::GetTimespanReport() { return TEXT("NProcGenTaskAnalytics::Timespans are not available in UE_BUILD_SHIPPING."); }
FString FNProcGenTaskAnalytics::GetCountersReport() { return TEXT("NProcGenTaskAnalytics::Counters are not available in UE_BUILD_SHIPPING.");}

int FNProcGenTaskAnalytics::CollectGenerationPassesCreate() { return -1; }
void FNProcGenTaskAnalytics::CollectGenerationPassesStart(int32 Index) {}
void FNProcGenTaskAnalytics::CollectGenerationPassesFinish(int32 Index) {}

void FNProcGenTaskAnalytics::SpawnCellProxiesStart() {}
void FNProcGenTaskAnalytics::SpawnCellProxiesFinish() {}

void FNProcGenTaskAnalytics::ProcGenFinalizeStart() {}
void FNProcGenTaskAnalytics::ProcGenFinalizeFinish() {}

#endif // !UE_BUILD_SHIPPING	