// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/NAssemblyTaskAnalytics.h"

#include "Developer/NReportListBlock.h"

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

void FNAssemblyTaskAnalytics::OrganGraphBuilder_AddMessages(int32 Index, const TArray<FString>& Messages)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.IterationMessages[Analytic.IterationsIndex].Append(Messages);
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

void FNAssemblyTaskAnalytics::OrganGraphBuilder_DiscardDueToNonFinisherConstraint(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.DiscardDueToNonFinisherConstraint.Increment();
}

void FNAssemblyTaskAnalytics::OrganGraphBuilder_CappedWithFinisher(int32 Index, int32 Value)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.CappedWithFinisher.Add(Value);
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


void FNAssemblyTaskAnalytics::AddToReports(FNReport* Report, TArray<FNReport>& DetailedReports)
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
	OrganBuilderTable->SetHeading("FNOrganGraphBuildTasks");
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
	ProcessPassTable->SetHeading("FNProcessPassTasks");
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
	SpawnCellProxiesTable->SetHeading("FNSpawnCellProxiesTask (Sliced)");
	SpawnCellProxiesTable->Initialize({ "Thread", "Spawns", "ms" });
	for (const auto Analytic : SpawnCellProxiesAnalytics)
	{
		SpawnCellProxiesTable->AddRow({ "Game", FString::FromInt(Analytic.Spawned.Num()), FString::SanitizeFloat(Analytic.Timer.Duration)});
		DurationTotal += Analytic.Timer.Duration;
		LoopTotal += Analytic.Timer.Duration;
	}
	
	OverviewTable = Report->GetTableBlock(OverviewTableTicket);
	OverviewTable->AddRow({"Game", "Spawn Cells (Sliced)", FString::SanitizeFloat(LoopTotal)});
	
	Report->AddReplaceToken("{{RUNTIME}}",  FString::SanitizeFloat(DurationTotal));
	
	
	for (const auto Analytic : OrganGraphBuilderAnalytics)
	{
		FNReport& DetailedReport = DetailedReports.AddDefaulted_GetRef();
		
		DetailedReport.SetDesiredFileName(Analytic.Name.Replace(TEXT(":"), TEXT("_"))); // Prepopulate Organ Name
		
		const int32 OrganOverviewTicket = DetailedReport.CreateContentBlock();
		FNReportContentBlock* OrganOverviewContentBlock = DetailedReport.GetContentBlock(OrganOverviewTicket);
		OrganOverviewContentBlock->SetHeading(Analytic.Name);
		OrganOverviewContentBlock->SetHeader(FString::Printf(TEXT("%i Iteration"), Analytic.Iterations));
		
		// Counter Table
		const int32 CounterTableTicket = DetailedReport.CreateTableBlock(OrganOverviewTicket);
		FNReportTableBlock* CountersTableBlock = DetailedReport.GetTableBlock(CounterTableTicket);
		CountersTableBlock->SetHeading("Counters");
		CountersTableBlock->Initialize({ "Iteration", "Null Nodes", "Cell Nodes", "Finisher Capped", 
			"False Start / Out Of Bounds", "False Start / World Colliding", 
			"Bad Placement / Intersecting", "Bad Placement / World Colliding", "Bad Placement / Existing Node World", "Bad Placement / Out Of Bounds", 
			"Constraint / Non-Finisher" });
		
		// Message Logs
		const int32 IterationMessagesTicket = DetailedReport.CreateListBlock(OrganOverviewTicket);
		FNReportListBlock* IterationMessagesContentBlock = DetailedReport.GetListBlock(IterationMessagesTicket);
		IterationMessagesContentBlock->SetHeading("Message Log");
		
		for (int32 i = 0; i < Analytic.Iterations; i++)
		{
			CountersTableBlock->AddRow({ 
				FString::FromInt(i),
				FString::FromInt(Analytic.AddNullNodes.Counter[i]), 
				FString::FromInt(Analytic.AddCellNodes.Counter[i]),
				FString::FromInt(Analytic.CappedWithFinisher.Counter[i]),
				FString::FromInt(Analytic.DiscardOutOfBoundsStart.Counter[i]), 
				FString::FromInt(Analytic.DiscardWorldCollidingStart.Counter[i]),
				FString::FromInt(Analytic.DiscardIntersectingCellNode.Counter[i]), 
				FString::FromInt(Analytic.DiscardWorldCollidingCellNode.Counter[i]), 
				FString::FromInt(Analytic.DiscardExistingNodeWorldCollidingCellNode.Counter[i]), 
				FString::FromInt(Analytic.DiscardOutOfBoundsCellNode.Counter[i]),
				FString::FromInt(Analytic.DiscardDueToNonFinisherConstraint.Counter[i])
			});
			
			for (const FString& Message : Analytic.IterationMessages[i])
			{
				IterationMessagesContentBlock->AddItem(FString::Printf(TEXT("%i: %s"), i, *Message));
			}
		}
	}
	
	
}

float FNAssemblyTaskAnalytics::GetTotalDuration()
{
	double DurationTotal = TaskGraphCreationTimer.Duration + CreateVirtualWorldContextTimer.Duration + 
		ProcessVirtualWorldContextTimer.Duration + CreateSpawnCellsContextTimer.Duration;
	
	for (const auto Analytic : OrganGraphBuilderAnalytics)
	{
		DurationTotal += Analytic.Timer.Duration;
	}
	for (const auto Analytic : ProcessPassAnalytics)
	{
		DurationTotal += Analytic.Timer.Duration;
	}
	for (const auto Analytic : SpawnCellProxiesAnalytics)
	{
		DurationTotal += Analytic.Timer.Duration;
	}
	return DurationTotal;
}

int FNAssemblyTaskAnalytics::GetSpawnedCellProxiesCount()
{
	int Count = 0;
	for (const auto Analytic : SpawnCellProxiesAnalytics)
	{
		Count += Analytic.Spawned.Num();
	}
	return Count;
}

void FNAssemblyTaskAnalytics::OrganGraphBuilderFinish(int32 Index)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.Timer.Stop();
}
