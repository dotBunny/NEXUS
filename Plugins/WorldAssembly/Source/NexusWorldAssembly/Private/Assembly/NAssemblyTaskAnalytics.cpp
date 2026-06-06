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
	Analytic.MinimumCellCount = MinimumCellCount;
	Analytic.MaximumCellCount = MaximumCellCount;
	Analytic.MaximumRetryCount = MaximumRetryCount;
}

void FNAssemblyTaskAnalytics::OrganGraphBuilder_SetResult(int32 Index, bool bSucceeded, int32 FinalCellNodeCount)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.bSucceeded = bSucceeded;
	Analytic.FinalCellNodeCount = FinalCellNodeCount;
}

void FNAssemblyTaskAnalytics::OrganGraphBuilder_SetFailure(int32 Index, int32 FinalCellNodeCount, const FString& Reason)
{
	FNOrganGraphBuilderAnalytics& Analytic = OrganGraphBuilderAnalytics[Index];
	Analytic.bSucceeded = false;
	Analytic.FinalCellNodeCount = FinalCellNodeCount;
	Analytic.FailureReasonOverride = Reason;
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


void FNAssemblyTaskAnalytics::AddToReports(FNReport* Report)
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
	int32 SucceededOrganCount = 0;
	const int32 OrganBuilderTableTicket = Report->CreateTableBlock(TimespanContentTicket);
	FNReportTableBlock* OrganBuilderTable = Report->GetTableBlock(OrganBuilderTableTicket);
	OrganBuilderTable->SetHeading("FNOrganGraphBuildTasks");
	OrganBuilderTable->Initialize({ "Thread", "Organ", "Iterations", "Status", "ms" });
	for (const auto Analytic : OrganGraphBuilderAnalytics)
	{
		OrganBuilderTable->AddRow({"Task", *Analytic.Name, FString::FromInt(Analytic.Iterations), Analytic.GetStatusString(), FString::SanitizeFloat(Analytic.Timer.Duration)});
		DurationTotal += Analytic.Timer.Duration;
		LoopTotal += Analytic.Timer.Duration;
		if (Analytic.bSucceeded) { SucceededOrganCount++; }
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

	// Headline the overall verdict: on the Analytics block header, in the operation overview's {{STATUS}} cell,
	// and -- when anything failed -- in a dedicated Failures block floated to the very top of the report.
	const int32 OrganCount = OrganGraphBuilderAnalytics.Num();
	const int32 FailedOrganCount = OrganCount - SucceededOrganCount;

	AnalyticsContentBlock = Report->GetContentBlock(AnalyticsContentTicket);
	if (FailedOrganCount > 0)
	{
		AnalyticsContentBlock->SetHeader(FString::Printf(TEXT("%i/%i organs succeeded - %i failed"), SucceededOrganCount, OrganCount, FailedOrganCount));
		Report->AddReplaceToken("{{STATUS}}", FString::Printf(TEXT("%i/%i FAILED"), FailedOrganCount, OrganCount));
	}
	else
	{
		AnalyticsContentBlock->SetHeader(FString::Printf(TEXT("%i/%i organs succeeded"), SucceededOrganCount, OrganCount));
		Report->AddReplaceToken("{{STATUS}}", FString::Printf(TEXT("OK (%i/%i)"), SucceededOrganCount, OrganCount));
	}

	// Format a dominant-rejection diagnosis the same way for both the Failures and Strained Successes recaps.
	auto FormatLikelyCause = [](const FNOrganGraphBuilderAnalytics& Analytic) -> FString
	{
		FString DominantReason;
		int32 RejectCount = 0;
		int32 TotalAttempts = 0;
		if (!Analytic.GetDominantRejection(DominantReason, RejectCount, TotalAttempts))
		{
			return FString(TEXT("-"));
		}
		const int32 Percent = TotalAttempts > 0 ? FMath::RoundToInt((RejectCount * 100.0) / TotalAttempts) : 0;
		return FString::Printf(TEXT("%s (%i, %i%%)"), *DominantReason, RejectCount, Percent);
	};

	if (FailedOrganCount > 0)
	{
		// Negative priority floats this above the operation and analytics blocks so failures are the first thing read.
		const int32 FailuresContentTicket = Report->CreateContentBlock(0, -100);
		FNReportContentBlock* FailuresContentBlock = Report->GetContentBlock(FailuresContentTicket);
		FailuresContentBlock->SetHeading("Failures");
		FailuresContentBlock->SetHeader(FString::Printf(TEXT("%i of %i organs failed to generate"), FailedOrganCount, OrganCount));

		const int32 FailuresTableTicket = Report->CreateTableBlock(FailuresContentTicket);
		FNReportTableBlock* FailuresTable = Report->GetTableBlock(FailuresTableTicket);
		FailuresTable->Initialize({ "Organ", "Cells", "Likely Cause", "Reason" });
		for (const auto Analytic : OrganGraphBuilderAnalytics)
		{
			if (Analytic.bSucceeded) continue;

			FString CellSummary = FString::FromInt(Analytic.FinalCellNodeCount);
			if (Analytic.MinimumCellCount > 0)
			{
				CellSummary = FString::Printf(TEXT("%i / min %i"), Analytic.FinalCellNodeCount, Analytic.MinimumCellCount);
			}

			// Turn the per-iteration discard counters into a single dominant cause with its share of placement attempts.
			FString Reason = Analytic.GetFailureReason();
			if (Reason.IsEmpty()) { Reason = TEXT("No diagnostics captured (early bailout)."); }

			FailuresTable->AddRow({ *Analytic.Name, CellSummary, FormatLikelyCause(Analytic), Reason });
		}
	}

	// Recap organs that succeeded only after consuming retries -- same diagnosis surface as Failures, so the cost of a
	// strained build is visible at a glance without opening the per-organ detailed report.
	int32 StrainedSuccessCount = 0;
	for (const FNOrganGraphBuilderAnalytics& Analytic : OrganGraphBuilderAnalytics)
	{
		if (Analytic.bSucceeded && Analytic.Iterations > 1) { StrainedSuccessCount++; }
	}
	if (StrainedSuccessCount > 0)
	{
		// Priority below the Failures block (-100) but above the regular report body (0).
		const int32 RetriesContentTicket = Report->CreateContentBlock(0, -50);
		FNReportContentBlock* RetriesContentBlock = Report->GetContentBlock(RetriesContentTicket);
		RetriesContentBlock->SetHeading("Strained Successes");
		RetriesContentBlock->SetHeader(FString::Printf(TEXT("%i organ(s) needed retries before succeeding"), StrainedSuccessCount));

		const int32 RetriesTableTicket = Report->CreateTableBlock(RetriesContentTicket);
		FNReportTableBlock* RetriesTable = Report->GetTableBlock(RetriesTableTicket);
		RetriesTable->Initialize({ "Organ", "Retries", "Likely Cause" });
		for (const auto Analytic : OrganGraphBuilderAnalytics)
		{
			if (!Analytic.bSucceeded || Analytic.Iterations <= 1) continue;

			const FString RetrySummary = FString::Printf(TEXT("%i / %i"), Analytic.Iterations - 1, Analytic.MaximumRetryCount);
			RetriesTable->AddRow({ *Analytic.Name, RetrySummary, FormatLikelyCause(Analytic) });
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
