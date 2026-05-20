// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/NAssemblyTaskAnalytics.h"

void FNAssemblyTaskAnalytics::TaskGraphCreationStart()
{
	TaskGraphCreationTimer.Start();
};
void FNAssemblyTaskAnalytics::TaskGraphCreationFinish()
{
	TaskGraphCreationTimer.Stop();
}
void FNAssemblyTaskAnalytics::CreateVirtualWorldContextStart()
{
	CreateVirtualWorldContextTimer.Start();
}
void FNAssemblyTaskAnalytics::CreateVirtualWorldContextFinish()
{
	CreateVirtualWorldContextTimer.Stop();
}
void FNAssemblyTaskAnalytics::ProcessVirtualWorldContextStart()
{
	ProcessVirtualWorldContextTimer.Start();
}

void FNAssemblyTaskAnalytics::ProcessVirtualWorldContextFinish()
{
	ProcessVirtualWorldContextTimer.Stop();
}

int32 FNAssemblyTaskAnalytics::OrganGraphBuilderCreate()
{
	OrganGraphBuilderAnalytics.Add(FNOrganGraphBuilderAnalytics());
	return OrganGraphBuilderAnalytics.Num() - 1;
}
void FNAssemblyTaskAnalytics::OrganGraphBuilderStart(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.Timer.Start();
}
void FNAssemblyTaskAnalytics::OrganGraphBuilder_Init(int32 Index, const FString& Name, int32 MinimumCellCount, int32 MaximumCellCount, int32 MaximumRetryCount)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.Name = Name;
}
void FNAssemblyTaskAnalytics::OrganGraphBuilder_AddNullNode(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.AddNullNodes.Increment();
}
void FNAssemblyTaskAnalytics::OrganGraphBuilder_AddCellNode(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.AddCellNodes.Increment();
}
void FNAssemblyTaskAnalytics::OrganGraphBuilder_DiscardOutOfBoundsStart(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.DiscardOutOfBoundsStart.Increment();
}
void FNAssemblyTaskAnalytics::OrganGraphBuilder_DiscardWorldCollidingStart(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.DiscardWorldCollidingStart.Increment();
}
void FNAssemblyTaskAnalytics::OrganGraphBuilder_DiscardOutOfBoundsCellNode(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.DiscardOutOfBoundsCellNode.Increment();
}
void FNAssemblyTaskAnalytics::OrganGraphBuilder_DiscardIntersectingCellNode(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.DiscardIntersectingCellNode.Increment();
}
void FNAssemblyTaskAnalytics::OrganGraphBuilder_DiscardWorldCollidingCellNode(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.DiscardWorldCollidingCellNode.Increment();
}

void FNAssemblyTaskAnalytics::OrganGraphBuilder_DiscardExistingNodeWorldCollidingCellNode(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.DiscardExistingNodeWorldCollidingCellNode.Increment();
}

void FNAssemblyTaskAnalytics::OrganGraphBuilder_NextIteration(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.NextIteration();
}

int32 FNAssemblyTaskAnalytics::CollectGenerationPassesCreate()
{
	ProcessPassAnalytics.Add(FNProcessPassAnalytics());
	const int32 Phase = ProcessPassAnalytics.Num() - 1;
	ProcessPassAnalytics.Last().Phase = Phase;
	return Phase;
}

void FNAssemblyTaskAnalytics::CollectGenerationPassesStart(int32 Index)
{
	FNProcessPassAnalytics& Analytic = ProcessPassAnalytics[Index];
	Analytic.Timer.Start();
}

void FNAssemblyTaskAnalytics::CollectGenerationPassesFinish(int32 Index)
{
	FNProcessPassAnalytics& Analytic = ProcessPassAnalytics[Index];
	Analytic.Timer.Stop();
}

void FNAssemblyTaskAnalytics::CreateSpawnCellsContextStart()
{
	CreateSpawnCellsContextTimer.Start();
}

void FNAssemblyTaskAnalytics::CreateSpawnCellsContextFinish()
{
	CreateSpawnCellsContextTimer.Stop();
}

int32 FNAssemblyTaskAnalytics::SpawnCellProxiesCreate()
{
	SpawnCellProxiesAnalytics.Add(FNSpawnCellProxiesAnalytics());
	return SpawnCellProxiesAnalytics.Num() - 1;
}

void FNAssemblyTaskAnalytics::SpawnCellProxiesStart(int32 Index)
{
	FNSpawnCellProxiesAnalytics& Analytic = SpawnCellProxiesAnalytics[Index];
	Analytic.Timer.Start();
}

void FNAssemblyTaskAnalytics::SpawnCellProxiesSpawned(int32 Index, FName Template)
{
	FNSpawnCellProxiesAnalytics& Analytic = SpawnCellProxiesAnalytics[Index];
	Analytic.Spawned.Add(Template);
}


void FNAssemblyTaskAnalytics::SpawnCellProxiesFinish(int32 Index)
{
	FNSpawnCellProxiesAnalytics& Analytic = SpawnCellProxiesAnalytics[Index];
	Analytic.Timer.Stop();
}


void FNAssemblyTaskAnalytics::AddToReport(FNReport* Report)
{
	const int32 AnalyticsContentTicket = Report->CreateContentBlock();
	FNReportContentBlock* AnalyticsContentBlock = Report->GetContentBlock(AnalyticsContentTicket);
	AnalyticsContentBlock->SetHeading("Analytics");
	
	double DurationTotal = TaskGraphCreationTimer.Duration + CreateVirtualWorldContextTimer.Duration + 
		ProcessVirtualWorldContextTimer.Duration + CreateSpawnCellsContextTimer.Duration;
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
	
	Report->AddReplaceToken("{{RUNTIME}}",  FString::SanitizeFloat(DurationTotal));
}

void FNAssemblyTaskAnalytics::OrganGraphBuilderFinish(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.Timer.Stop();
}
