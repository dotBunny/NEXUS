// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Generation/NProcGenTaskAnalytics.h"

void FNProcGenTaskAnalytics::TaskGraphCreationStart()
{
	TaskGraphCreationTimer.Start();
};
void FNProcGenTaskAnalytics::TaskGraphCreationFinish()
{
	TaskGraphCreationTimer.Stop();
}
void FNProcGenTaskAnalytics::CreateVirtualWorldContextStart()
{
	CreateVirtualWorldContextTimer.Start();
}
void FNProcGenTaskAnalytics::CreateVirtualWorldContextFinish()
{
	CreateVirtualWorldContextTimer.Stop();
}
void FNProcGenTaskAnalytics::ProcessVirtualWorldContextStart()
{
	ProcessVirtualWorldContextTimer.Start();
}

void FNProcGenTaskAnalytics::ProcessVirtualWorldContextFinish()
{
	ProcessVirtualWorldContextTimer.Stop();
}

int32 FNProcGenTaskAnalytics::OrganGraphBuilderCreate()
{
	OrganGraphBuilderAnalytics.Add(FNOrganGraphBuilderAnalytics());
	return OrganGraphBuilderAnalytics.Num() - 1;
}
void FNProcGenTaskAnalytics::OrganGraphBuilderStart(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.Timer.Start();
}
void FNProcGenTaskAnalytics::OrganGraphBuilder_Init(int32 Index, const FString& Name, int32 MinimumCellCount, int32 MaximumCellCount, int32 MaximumRetryCount)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.Name = Name;
}
void FNProcGenTaskAnalytics::OrganGraphBuilder_AddNullNode(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.AddNullNodes.Increment();
}
void FNProcGenTaskAnalytics::OrganGraphBuilder_AddCellNode(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.AddCellNodes.Increment();
}
void FNProcGenTaskAnalytics::OrganGraphBuilder_DiscardOutOfBoundsStart(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.DiscardOutOfBoundsStart.Increment();
}
void FNProcGenTaskAnalytics::OrganGraphBuilder_DiscardWorldCollidingStart(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.DiscardWorldCollidingStart.Increment();
}
void FNProcGenTaskAnalytics::OrganGraphBuilder_DiscardOutOfBoundsCellNode(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.DiscardOutOfBoundsCellNode.Increment();
}
void FNProcGenTaskAnalytics::OrganGraphBuilder_DiscardIntersectingCellNode(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.DiscardIntersectingCellNode.Increment();
}
void FNProcGenTaskAnalytics::OrganGraphBuilder_DiscardWorldCollidingCellNode(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.DiscardWorldCollidingCellNode.Increment();
}

void FNProcGenTaskAnalytics::OrganGraphBuilder_DiscardExistingNodeWorldCollidingCellNode(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.DiscardExistingNodeWorldCollidingCellNode.Increment();
}

void FNProcGenTaskAnalytics::OrganGraphBuilder_NextIteration(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.NextIteration();
}

int32 FNProcGenTaskAnalytics::CollectGenerationPassesCreate()
{
	ProcessPassAnalytics.Add(FNProcessPassAnalytics());
	const int32 Phase = ProcessPassAnalytics.Num() - 1;
	ProcessPassAnalytics.Last().Phase = Phase;
	return Phase;
}

void FNProcGenTaskAnalytics::CollectGenerationPassesStart(int32 Index)
{
	FNProcessPassAnalytics& Analytic = ProcessPassAnalytics[Index];
	Analytic.Timer.Start();
}

void FNProcGenTaskAnalytics::CollectGenerationPassesFinish(int32 Index)
{
	FNProcessPassAnalytics& Analytic = ProcessPassAnalytics[Index];
	Analytic.Timer.Stop();
}

void FNProcGenTaskAnalytics::CreateSpawnCellsContextStart()
{
	CreateSpawnCellsContextTimer.Start();
}

void FNProcGenTaskAnalytics::CreateSpawnCellsContextFinish()
{
	CreateSpawnCellsContextTimer.Stop();
}

int32 FNProcGenTaskAnalytics::SpawnCellProxiesCreate()
{
	SpawnCellProxiesAnalytics.Add(FNSpawnCellProxiesAnalytics());
	return SpawnCellProxiesAnalytics.Num() - 1;
}

void FNProcGenTaskAnalytics::SpawnCellProxiesStart(int32 Index)
{
	FNSpawnCellProxiesAnalytics& Analytic = SpawnCellProxiesAnalytics[Index];
	Analytic.Timer.Start();
}

void FNProcGenTaskAnalytics::SpawnCellProxiesSpawned(int32 Index, FName Template)
{
	FNSpawnCellProxiesAnalytics& Analytic = SpawnCellProxiesAnalytics[Index];
	Analytic.Spawned.Add(Template);
}


void FNProcGenTaskAnalytics::SpawnCellProxiesFinish(int32 Index)
{
	FNSpawnCellProxiesAnalytics& Analytic = SpawnCellProxiesAnalytics[Index];
	Analytic.Timer.Stop();
}

void FNProcGenTaskAnalytics::ProcGenFinalizeStart()
{
	ProcGenFinalizeTimer.Start();
}

void FNProcGenTaskAnalytics::ProcGenFinalizeFinish()
{
	ProcGenFinalizeTimer.Stop();
}


FNReport FNProcGenTaskAnalytics::GetReport()
{
	FNReport Report = FNReport("FNProcGenTaskAnalytics");
	
	double DurationTotal = 
		TaskGraphCreationTimer.Duration + 
		CreateVirtualWorldContextTimer.Duration + 
		ProcessVirtualWorldContextTimer.Duration;

	const int32 TimespanContentTicket = Report.CreateContentBlock();
	FNReportContentBlock* TimespanContentBlock = Report.GetContentBlock(TimespanContentTicket);
	TimespanContentBlock->SetHeading("Timespans");

	const int32 OverviewTableTicket = Report.CreateTableBlock(TimespanContentTicket);
	FNReportTableBlock* OverviewTable = Report.GetTableBlock(OverviewTableTicket);
	OverviewTable->SetHeading("Overview");
	OverviewTable->Initialize({ "Thread", "Task", "ms" });
	OverviewTable->AddRow({"Game", "TaskGraph Creation", FString::SanitizeFloat(TaskGraphCreationTimer.Duration)});
	OverviewTable->AddRow({"Game", "Create VirtualWorldContext", FString::SanitizeFloat(CreateVirtualWorldContextTimer.Duration)});
	OverviewTable->AddRow({"Off", "Process VirtualWorldContext", FString::SanitizeFloat(ProcessVirtualWorldContextTimer.Duration)});
	
	return MoveTemp(Report);
}

FString FNProcGenTaskAnalytics::GetTimespanReport()
{
	FStringBuilderBase Builder = FStringBuilderBase();
	
	Builder.Appendf(TEXT("[%s] Timespans\n"), *DisplayName.ToString());
	
	// Report Timespans
	double DurationTotal = 0;
	
	Builder.Appendf(TEXT("[G]\tTaskGraph Creation: %f ms\n"), TaskGraphCreationTimer.Duration);
	DurationTotal += TaskGraphCreationTimer.Duration;
	
	Builder.Appendf(TEXT("[G]\tCreate VirtualWorldContext: %f ms\n"), CreateVirtualWorldContextTimer.Duration);
	DurationTotal += CreateVirtualWorldContextTimer.Duration;
	
	Builder.Appendf(TEXT("[B]\tProcess VirtualWorldContext: %f ms\n"), ProcessVirtualWorldContextTimer.Duration);
	DurationTotal += ProcessVirtualWorldContextTimer.Duration;
	
	double OrganGraphBuilderDurationTotal = 0;
	Builder.Append(TEXT("\tOrganGraph Builders:\n"));
	for (const auto Analytic : OrganGraphBuilderAnalytics)
	{
		Builder.Appendf(TEXT("[B]\t\t%s (%i): %f ms\n"), *Analytic.Name, Analytic.Iterations, Analytic.Timer.Duration);
		DurationTotal += Analytic.Timer.Duration;
		OrganGraphBuilderDurationTotal += Analytic.Timer.Duration;
	}
	Builder.Appendf(TEXT("\t\tTotal=%f ms:\n"), OrganGraphBuilderDurationTotal);
	
	Builder.Append(TEXT("\tProcess Passes:\n"));
	for (const auto Analytic : ProcessPassAnalytics)
	{
		Builder.Appendf(TEXT("[B]\t\tPass %i: %f ms\n"), Analytic.Phase, Analytic.Timer.Duration);
		DurationTotal += Analytic.Timer.Duration;
	}
	
	Builder.Appendf(TEXT("[B]\tCreate SpawnCellsContext: %f\n"), CreateSpawnCellsContextTimer.Duration);
	
	DurationTotal += CreateSpawnCellsContextTimer.Duration;
	
	double SpawnCellProxiesDurationTotal = 0;
	Builder.Append(TEXT("\tSpawn Cell Proxies (Sliced):\n"));
	for (const auto Analytic : SpawnCellProxiesAnalytics)
	{
		Builder.Appendf(TEXT("[G]\t\t%i Spawns in %f ms\n"), Analytic.Spawned.Num(), Analytic.Timer.Duration);
		DurationTotal +=  Analytic.Timer.Duration;
		SpawnCellProxiesDurationTotal +=  Analytic.Timer.Duration;
	}
	Builder.Appendf(TEXT("\t\tTotal=%f ms:\n"), SpawnCellProxiesDurationTotal);
	
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
		for (int32 i = 0; i < OrganGraphBuilderAnalytic.Iterations; i++)
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
			Builder.Appendf(TEXT("\t\t\t\tExisting Node World Colliding: %i\n"), OrganGraphBuilderAnalytic.DiscardExistingNodeWorldCollidingCellNode.Counter[i]);
			Builder.Appendf(TEXT("\t\t\t\tOut Of Bounds: %i\n"), OrganGraphBuilderAnalytic.DiscardOutOfBoundsCellNode.Counter[i]);
		}
	}
	
	return Builder.ToString();
}


void FNProcGenTaskAnalytics::OrganGraphBuilderFinish(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.Timer.Stop();
}
