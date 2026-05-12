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


void FNProcGenTaskAnalytics::AddToReport(FNReport* Report)
{
	const int32 AnalyticsContentTicket = Report->CreateContentBlock();
	FNReportContentBlock* AnalyticsContentBlock = Report->GetContentBlock(AnalyticsContentTicket);
	AnalyticsContentBlock->SetHeading("Analytics");
	
	double DurationTotal = TaskGraphCreationTimer.Duration + CreateVirtualWorldContextTimer.Duration + 
		ProcessVirtualWorldContextTimer.Duration + CreateSpawnCellsContextTimer.Duration + ProcGenFinalizeTimer.Duration;
	double LoopTotal = 0;

	const int32 TimespanContentTicket = Report->CreateContentBlock(AnalyticsContentTicket);
	FNReportContentBlock* TimespanContentBlock = Report->GetContentBlock(TimespanContentTicket);
	TimespanContentBlock->SetHeading("Timespans");

	// Start Creating Overview
	const int32 OverviewTableTicket = Report->CreateTableBlock(TimespanContentTicket);
	FNReportTableBlock* OverviewTable = Report->GetTableBlock(OverviewTableTicket);
	OverviewTable->SetHeading("Overview");
	OverviewTable->Initialize({ "Thread", "Task", "ms" });
	OverviewTable->AddRow({"Game", "TaskGraph Creation", FString::SanitizeFloat(TaskGraphCreationTimer.Duration)});
	OverviewTable->AddRow({"Game", "Create VirtualWorldContext", FString::SanitizeFloat(CreateVirtualWorldContextTimer.Duration)});
	OverviewTable->AddRow({"Task", "Process VirtualWorldContext", FString::SanitizeFloat(ProcessVirtualWorldContextTimer.Duration)});
	
	// Organ Builders Individual Times
	LoopTotal = 0;
	const int32 OrganBuilderTableTicket = Report->CreateTableBlock(TimespanContentTicket);
	FNReportTableBlock* OrganBuilderTable = Report->GetTableBlock(OrganBuilderTableTicket);
	OrganBuilderTable->SetHeading("Organs");
	OrganBuilderTable->Initialize({ "Thread", "Organ", "Iterations", "ms" });
	for (const auto Analytic : OrganGraphBuilderAnalytics)
	{
		OrganBuilderTable->AddRow({"Task", *Analytic.Name, FString::FromInt(Analytic.Iterations), FString::SanitizeFloat(Analytic.Timer.Duration)});
		DurationTotal += Analytic.Timer.Duration;
		LoopTotal += Analytic.Timer.Duration;
	}
	
	// Need to re-get as it may have moved
	OverviewTable = Report->GetTableBlock(OverviewTableTicket);
	OverviewTable->AddRow({"Task", "OrganGraph Builders", FString::SanitizeFloat(LoopTotal)});
	
	// Collection timings
	LoopTotal = 0;
	const int32 ProcessPassTableTicket = Report->CreateTableBlock(TimespanContentTicket);
	FNReportTableBlock* ProcessPassTable = Report->GetTableBlock(ProcessPassTableTicket);
	ProcessPassTable->SetHeading("Collection Passes");
	ProcessPassTable->Initialize({ "Thread", "Phase", "ms" });
	for (const auto Analytic : ProcessPassAnalytics)
	{
		ProcessPassTable->AddRow({"Task", FString::FromInt(Analytic.Phase), FString::SanitizeFloat(Analytic.Timer.Duration)});
		DurationTotal += Analytic.Timer.Duration;
		LoopTotal += Analytic.Timer.Duration;
	}
	
	// Need to re-get as it may have moved
	OverviewTable = Report->GetTableBlock(OverviewTableTicket);
	OverviewTable->AddRow({"Task", "Process Pass", FString::SanitizeFloat(LoopTotal)});
	OverviewTable->AddRow({"Task", "Create SpawnCellsContext", FString::SanitizeFloat(CreateSpawnCellsContextTimer.Duration)});

	LoopTotal = 0;
	const int32 SpawnCellProxiesTableTicket = Report->CreateTableBlock(TimespanContentTicket);
	FNReportTableBlock* SpawnCellProxiesTable = Report->GetTableBlock(SpawnCellProxiesTableTicket);
	SpawnCellProxiesTable->SetHeading("Spawn Cells (Sliced)");
	SpawnCellProxiesTable->Initialize({ "Thread", "Spawns", "ms" });
	for (const auto Analytic : SpawnCellProxiesAnalytics)
	{
		SpawnCellProxiesTable->AddRow({ "Game", FString::FromInt(Analytic.Spawned.Num()), FString::SanitizeFloat(Analytic.Timer.Duration)});
		DurationTotal += Analytic.Timer.Duration;
		LoopTotal += Analytic.Timer.Duration;
	}
	
	// Need to re-get as it may have moved
	OverviewTable = Report->GetTableBlock(OverviewTableTicket);
	OverviewTable->AddRow({"Game", "Spawn Cells (Sliced)", FString::SanitizeFloat(LoopTotal)});
	OverviewTable->AddRow({"Game", "ProcGen Finalize", FString::SanitizeFloat(ProcGenFinalizeTimer.Duration)});
	
	
	const int32 CountersContentTicket = Report->CreateContentBlock(AnalyticsContentTicket);
	FNReportContentBlock* CountersContentBlock = Report->GetContentBlock(CountersContentTicket);
	CountersContentBlock->SetHeading("Counters");
	
	for (const auto Analytic : OrganGraphBuilderAnalytics)
	{
		const int32 OrganContentTicket = Report->CreateContentBlock(CountersContentTicket);
		FNReportContentBlock* OrganContentBlock = Report->GetContentBlock(OrganContentTicket);
		OrganContentBlock->SetHeading(Analytic.Name);
		
		for (int32 i = 0; i < Analytic.Iterations; i++)
		{
			const int32 FalseStartTicket = Report->CreateTableBlock(OrganContentTicket);
			FNReportTableBlock* FalseStartTable = Report->GetTableBlock(FalseStartTicket);
			FalseStartTable->SetHeading("False Starts");
			FalseStartTable->Initialize({ "Out Of Bounds", "World Colliding" });
			FalseStartTable->AddRow({ FString::FromInt(Analytic.DiscardOutOfBoundsStart.Counter[i]), FString::FromInt(Analytic.DiscardWorldCollidingStart.Counter[i]) });
		
			const int32 AddingNodesTicket = Report->CreateTableBlock(OrganContentTicket);
			FNReportTableBlock* AddingNodesTable = Report->GetTableBlock(AddingNodesTicket);
			AddingNodesTable->SetHeading("Adding Nodes");
			AddingNodesTable->Initialize({ "Null", "Cell" });
			AddingNodesTable->AddRow({ FString::FromInt(Analytic.AddNullNodes.Counter[i]), FString::FromInt(Analytic.AddCellNodes.Counter[i]) });
		
			const int32 BadPlacementCellTicket = Report->CreateTableBlock(OrganContentTicket);
			FNReportTableBlock* BadPlacementCellTable = Report->GetTableBlock(BadPlacementCellTicket);
			BadPlacementCellTable->SetHeading("Bad Placement (Cell)");
			BadPlacementCellTable->Initialize({ "Intersecting", "World Colliding", "Existing Node World Colliding", "Out Of Bounds" });
			BadPlacementCellTable->AddRow({ FString::FromInt(Analytic.DiscardIntersectingCellNode.Counter[i]), FString::FromInt(Analytic.DiscardWorldCollidingCellNode.Counter[i]),
			FString::FromInt(Analytic.DiscardExistingNodeWorldCollidingCellNode.Counter[i]), FString::FromInt(Analytic.DiscardOutOfBoundsCellNode.Counter[i])});
		
		}
	}
}

void FNProcGenTaskAnalytics::OrganGraphBuilderFinish(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.Timer.Stop();
}
