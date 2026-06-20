// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorSubsystem.h"

#include "Assembly/NAssemblyOperation.h"
#include "Assembly/Contexts/NAssemblyOperationContext.h"
#include "Assembly/Contexts/NAssemblyTaskGraphContext.h"
#include "Cell/NCellProxy.h"
#include "Editor.h"
#include "TimerManager.h"
#include "NWorldAssemblyContextCache.h"
#include "NWorldAssemblyEditorCommands.h"
#include "NWorldAssemblyEditorMinimal.h"
#include "NWorldAssemblyEditorToolMenu.h"
#include "NWorldAssemblyEditorUserSettings.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Organ/NOrganComponent.h"
#include "Widgets/Notifications/SNotificationList.h"

void UNWorldAssemblyEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	OnMapLoadHandle = FEditorDelegates::OnMapLoad.AddUObject(this, &UNWorldAssemblyEditorSubsystem::OnMapLoad);
	PreBeginPIEHandle = FEditorDelegates::PreBeginPIE.AddUObject(this, &UNWorldAssemblyEditorSubsystem::OnPreBeginPIE);
}

void UNWorldAssemblyEditorSubsystem::Deinitialize()
{
	// Tear down the auto-assembly loop first so its timer can never fire into a half-deinitialized subsystem.
	StopAutoAssemblyLoop();

	// Clear cached persistent operation data
	if (CachedOperationTickets.Num() > 0)
	{
		FNWorldAssemblyContextCache::ClearContext(CachedOperationTickets);
		CachedOperationTickets.Empty();
	}

	if (HasGeneratedCellProxies())
	{
		ClearAllProxies();
	}

	if (OnMapLoadHandle.IsValid())
	{
		FEditorDelegates::OnMapLoad.Remove(OnMapLoadHandle);
		OnMapLoadHandle.Reset();
	}

	if (PreBeginPIEHandle.IsValid())
	{
		FEditorDelegates::PreBeginPIE.Remove(PreBeginPIEHandle);
		PreBeginPIEHandle.Reset();
	}


	// Stop all known operations
	for (int32 i = KnownOperations.Num() - 1; i >= 0; i--)
	{
		UNAssemblyOperation* Operation = KnownOperations[i];
		if (Operation->IsRunning())
		{
			Operation->Cancel();
		}
		else
		{
			Operation->TearDownOperation();
		}
	}

	Super::Deinitialize();
}


void UNWorldAssemblyEditorSubsystem::StartOperation(UNAssemblyOperation* Operation)
{
	if (Operation == nullptr || !Operation->Context.IsValid()) return;

	// Clear for anything in operation
	for (UNOrganComponent* Component :  Operation->Context->InputComponents)
	{
		int32 LastOperationTicket = Component->GetAndResetLastOperationTicket();
		if (LastOperationTicket != 0)
		{
			ClearGenerated(LastOperationTicket);
		}
	}

	KnownOperations.AddUnique(Operation);
	CachedOperationTickets.Add(Operation->GetTicket());
	Operation->StartBuild(this, this);
}

void UNWorldAssemblyEditorSubsystem::OnQuickAssemblyProgressChanged(float Progress)
{
	FNWorldAssemblyEditorToolMenu::SetQuickAssemblyProgress(Progress);
}

void UNWorldAssemblyEditorSubsystem::OnOperationFinished(UNAssemblyOperation* Operation, TSharedRef<FNAssemblyTaskGraphContext> TaskGraphContext)
{
	// A run belongs to an auto-assembly loop when it is the tracked quick-assembly operation and the loop is engaged.
	// Loop runs are folded into the summary instead of toasting one-per-run; standalone runs keep their own toast.
	const bool bIsQuickAssemblyOp = Operation->GetTicket() == FNWorldAssemblyEditorToolMenu::GetQuickAssemblyOperationTicket();
	const bool bWasAutoLoopRun = bIsQuickAssemblyOp && bAutoAssemblyLoopActive;
	if (bWasAutoLoopRun)
	{
		AccumulateAutoAssemblyResult(Operation->GetResult());
	}

	// Drop the tracked ticket for the quick-assembly operation we were following so the toolbar button can leave
	// its "operation running" state. This is the natural-completion path only; a cancelled operation routes
	// through OnOperationDestroyed instead and must never re-arm the loop.
	if (bIsQuickAssemblyOp)
	{
		FNWorldAssemblyEditorToolMenu::SetQuickAssemblyOperationTicket(-1);

		// Re-arm the inter-run timer when an auto-assembly loop is engaged and still enabled. Re-reading the live
		// setting here is what makes toggling Auto Assembly off mid-loop stop it gracefully after the current run.
		if (bAutoAssemblyLoopActive && UNWorldAssemblyEditorUserSettings::Get()->bQuickAssemblyAutoAssembly)
		{
			ScheduleNextAutoAssembly();
			// Keep the bar visible and reset it to 0 so the countdown to the next run reads from empty.
			FNWorldAssemblyEditorToolMenu::SetQuickAssemblyProgress(0.0f);
		}
		else
		{
			FNWorldAssemblyEditorToolMenu::ClearQuickAssemblyProgress();
			// Reaching here with the loop still active means Auto Assembly was toggled off mid-loop - a deliberate
			// user stop, so surface the accumulated summary. A standalone run (loop never active) emits nothing.
			StopAutoAssemblyLoop(/*bEmitSummary*/ bAutoAssemblyLoopActive);
		}
	}

	for (ANCellProxy* Proxy : TaskGraphContext->CreatedProxies)
	{
		KnownProxies.Add(Proxy);
	}

	// Make our own map to the created proxies tied to the operation ticket
	ProxyMap.Add(Operation->GetTicket(), TArray<TObjectPtr<ANCellProxy>>(TaskGraphContext->CreatedProxies));
	KnownOperations.Remove(Operation);

	// Auto-assembly loop runs are summarized on stop, so suppress their per-run toast here.
	if (bWasAutoLoopRun)
	{
		return;
	}

	// Toast
	const FNAssemblyOperationResult Results = Operation->GetResult();
	FNotificationInfo Info(Results.Title);
	Info.SubText = Results.Message;
	Info.ExpireDuration = 5.0f;
	Info.bFireAndForget = true;

	// Create Report Link
	if (!TaskGraphContext->ReportFilePath.IsEmpty())
	{
		FString FilePath = TaskGraphContext->ReportFilePath;
		Info.HyperlinkText = FText::FromString(FPaths::GetCleanFilename(FilePath));
		Info.Hyperlink = FSimpleDelegate::CreateLambda([FilePath]
		{
			FPlatformProcess::LaunchFileInDefaultExternalApplication(*FPaths::ConvertRelativePathToFull(FilePath));
		});
	}

	if (Results.bWarning)
	{
		Info.Image = FAppStyle::GetBrush("Icons.WarningWithColor");
	}
	else if (Results.bSuccess)
	{
		Info.Image = FAppStyle::GetBrush("Icons.SuccessWithColor");
	}
	else
	{
		Info.Image = FAppStyle::GetBrush("Icons.ErrorWithColor");
	}
	FSlateNotificationManager::Get().AddNotification(Info);
}

void UNWorldAssemblyEditorSubsystem::OnOperationDestroyed(UNAssemblyOperation* Operation)
{
	// Cancelled/torn down before finishing - make sure the progress bar does not linger and the toolbar button
	// reverts to its "start" state by dropping the tracked ticket.
	if (Operation->GetTicket() == FNWorldAssemblyEditorToolMenu::GetQuickAssemblyOperationTicket())
	{
		FNWorldAssemblyEditorToolMenu::ClearQuickAssemblyProgress();
		FNWorldAssemblyEditorToolMenu::SetQuickAssemblyOperationTicket(-1);
	}

	KnownOperations.Remove(Operation);
}

void UNWorldAssemblyEditorSubsystem::ClearAllProxies()
{
	// Bulk clears can tear down streamed sublevel actors the user may have selected; drop the entire
	// selection so the typed-element registry does not assert on a stale handle next mouse-move.
	if (GEditor != nullptr)
	{
		GEditor->SelectNone(false, true, false);
	}

	for (int32 i = 0; i < KnownProxies.Num(); i++)
	{
		if (IsValid(KnownProxies[i]))
		{
			KnownProxies[i]->DestroyLevelInstance(true, true);
			KnownProxies[i]->Destroy(true, false);
		}
	}
	ProxyMap.Empty();
	KnownProxies.Empty();
}

void UNWorldAssemblyEditorSubsystem::ClearGenerated(const int32& OperationTicket)
{
	ClearGeneratedProxies(OperationTicket);
}

void UNWorldAssemblyEditorSubsystem::ClearGeneratedProxies(const int32& OperationTicket)
{
	if (ProxyMap.Num() > 0 && ProxyMap.Contains(OperationTicket))
	{
		// Per-ticket clears can also tear down streamed sublevel actors the user may have selected;
		// drop the entire selection so the typed-element registry does not assert next mouse-move.
		if (GEditor != nullptr)
		{
			GEditor->SelectNone(false, true, false);
		}

		TArray<ANCellProxy*> ProxiesArray = *ProxyMap.Find(OperationTicket);
		const int32 FoundCount = ProxiesArray.Num();
		for (int32 i = 0; i < FoundCount; i++)
		{
			KnownProxies.Remove(ProxiesArray[i]);
			if (IsValid(ProxiesArray[i]))
			{
				ProxiesArray[i]->DestroyLevelInstance(true, true);
				ProxiesArray[i]->Destroy(true, false);
			}
		}
		ProxyMap.Remove(OperationTicket);
	}
}

void UNWorldAssemblyEditorSubsystem::LoadAllGeneratedProxies()
{
	for (int32 i = 0; i < KnownProxies.Num(); i++)
	{
		if (IsValid(KnownProxies[i]))
		{
			KnownProxies[i]->LoadLevelInstance();
		}
	}
}

void UNWorldAssemblyEditorSubsystem::LoadGeneratedProxies(const int32& OperationTicket)
{
	if (ProxyMap.Num() > 0 && ProxyMap.Contains(OperationTicket))
	{
		TArray<ANCellProxy*> ProxiesArray = *ProxyMap.Find(OperationTicket);
		const int32 FoundCount = ProxiesArray.Num();
		for (int32 i = 0; i < FoundCount; i++)
		{
			if (IsValid(ProxiesArray[i]))
			{
				ProxiesArray[i]->LoadLevelInstance();
			}
		}
	}
}

void UNWorldAssemblyEditorSubsystem::UnloadAllGeneratedProxies()
{
	if (ProxyMap.Num() > 0)
	{
		TArray<int32> KnownKeys;
		ProxyMap.GetKeys(KnownKeys);
		for (auto Key : KnownKeys)
		{
			UnloadGeneratedProxies(Key);
		}
	}
}

void UNWorldAssemblyEditorSubsystem::UnloadGeneratedProxies(const int32& OperationTicket)
{
	if (ProxyMap.Num() > 0 && ProxyMap.Contains(OperationTicket))
	{
		TArray<ANCellProxy*> ProxiesArray = *ProxyMap.Find(OperationTicket);
		const int32 FoundCount = ProxiesArray.Num();
		for (int32 i = 0; i < FoundCount; i++)
		{
			if (IsValid(ProxiesArray[i]))
			{
				ProxiesArray[i]->UnloadLevelInstance();
			}
		}
	}
}

void UNWorldAssemblyEditorSubsystem::OnPreBeginPIE([[maybe_unused]] bool bArg)
{
	// Entering PIE invalidates the editor-world organs the loop runs against, so disengage it before clearing proxies.
	StopAutoAssemblyLoop();
	ClearAllProxies();
}

void UNWorldAssemblyEditorSubsystem::OnMapLoad(const FString& String, FCanLoadMap& CanLoadMap)
{
	// A map change invalidates the loop's target organs; disengage it before clearing proxies.
	StopAutoAssemblyLoop();
	ClearAllProxies();
}

void UNWorldAssemblyEditorSubsystem::BeginAutoAssemblyLoop()
{
	// StartQuickAssembly re-enters this for every run of the loop (the inter-run timer re-arms through it), so only
	// clear the tally when the loop first engages - otherwise each run would wipe the accumulated summary.
	if (!bAutoAssemblyLoopActive)
	{
		AutoAssemblySummary.Reset();
	}
	bAutoAssemblyLoopActive = true;
}

void UNWorldAssemblyEditorSubsystem::StopAutoAssemblyLoop(bool bEmitSummary)
{
	const bool bWasActive = bAutoAssemblyLoopActive;
	bAutoAssemblyLoopActive = false;

	if (AutoAssemblyTimerHandle.IsValid())
	{
		GEditor->GetTimerManager()->ClearTimer(AutoAssemblyTimerHandle);
		AutoAssemblyTimerHandle.Invalidate();
	}

	// If we were waiting between runs (no live operation owns the bar), the countdown bar is ours to hide. When an
	// operation is still running the cancel path tears it down through OnOperationDestroyed instead.
	if (bWasActive && !FNWorldAssemblyEditorToolMenu::IsQuickAssemblyOperationRunning())
	{
		FNWorldAssemblyEditorToolMenu::ClearQuickAssemblyProgress();
	}

	// On a deliberate user stop, surface the accumulated pass/warn/fail summary before discarding it. Environment-driven
	// stops pass bEmitSummary=false and silently drop the tally.
	if (bEmitSummary && AutoAssemblySummary.TotalRuns > 0)
	{
		ShowAutoAssemblySummaryToast();
	}
	AutoAssemblySummary.Reset();
}

void UNWorldAssemblyEditorSubsystem::AccumulateAutoAssemblyResult(const FNAssemblyOperationResult& Result)
{
	AutoAssemblySummary.TotalRuns++;
	AutoAssemblySummary.TotalCreatedCells += Result.CreatedCells;
	AutoAssemblySummary.TotalDurationMs += Result.Duration;

	// Mirror the per-op toast icon precedence: a warning outranks success, and success outranks failure.
	if (Result.bWarning)
	{
		AutoAssemblySummary.WarningCount++;
	}
	else if (Result.bSuccess)
	{
		AutoAssemblySummary.PassCount++;
	}
	else
	{
		AutoAssemblySummary.FailCount++;
	}
}

void UNWorldAssemblyEditorSubsystem::ShowAutoAssemblySummaryToast() const
{
	// Echo the same tally to the log so it persists beyond the transient toast.
	UE_LOG(LogNexusWorldAssemblyEditor, Log,
		TEXT("Auto Assembly summary: %d runs - %d passed, %d warnings, %d failed (%d cells, %.0f ms)"),
		AutoAssemblySummary.TotalRuns, AutoAssemblySummary.PassCount, AutoAssemblySummary.WarningCount,
		AutoAssemblySummary.FailCount, AutoAssemblySummary.TotalCreatedCells, AutoAssemblySummary.TotalDurationMs);

	// Durations are accumulated in milliseconds. Quick-assembly runs are usually sub-second, so format adaptively
	// (ms under a second, seconds with one decimal under a minute, m/s above) to avoid collapsing to "0s".
	const double TotalMs = AutoAssemblySummary.TotalDurationMs;
	FText DurationText;
	if (TotalMs < 1000.0)
	{
		DurationText = FText::Format(NSLOCTEXT("NexusWorldAssemblyEditor", "AutoAssemblySummaryDurationMs", "{0} ms"),
			FText::AsNumber(FMath::RoundToInt(TotalMs)));
	}
	else if (TotalMs < 60000.0)
	{
		FNumberFormattingOptions SecondsFormat;
		SecondsFormat.MinimumFractionalDigits = 1;
		SecondsFormat.MaximumFractionalDigits = 1;
		DurationText = FText::Format(NSLOCTEXT("NexusWorldAssemblyEditor", "AutoAssemblySummaryDurationSeconds", "{0}s"),
			FText::AsNumber(TotalMs / 1000.0, &SecondsFormat));
	}
	else
	{
		// Round to whole seconds first so splitting into minutes/seconds can't surface a "1m 60s".
		const int32 TotalSeconds = FMath::RoundToInt(TotalMs / 1000.0);
		DurationText = FText::Format(NSLOCTEXT("NexusWorldAssemblyEditor", "AutoAssemblySummaryDurationMinutes", "{0}m {1}s"),
			FText::AsNumber(TotalSeconds / 60), FText::AsNumber(TotalSeconds % 60));
	}

	FNotificationInfo Info(FText::Format(
		NSLOCTEXT("NexusWorldAssemblyEditor", "AutoAssemblySummaryTitle", "Auto Assembly — {0} passed, {1} warnings, {2} failed"),
		FText::AsNumber(AutoAssemblySummary.PassCount),
		FText::AsNumber(AutoAssemblySummary.WarningCount),
		FText::AsNumber(AutoAssemblySummary.FailCount)));
	Info.SubText = FText::Format(
		NSLOCTEXT("NexusWorldAssemblyEditor", "AutoAssemblySummarySubText", "{0} runs · {1} cells created · {2}"),
		FText::AsNumber(AutoAssemblySummary.TotalRuns),
		FText::AsNumber(AutoAssemblySummary.TotalCreatedCells),
		DurationText);
	Info.ExpireDuration = 8.0f;
	Info.bFireAndForget = true;

	// Aggregate severity: any failure makes the whole session an error, otherwise any warning a warning, else success.
	if (AutoAssemblySummary.FailCount > 0)
	{
		Info.Image = FAppStyle::GetBrush("Icons.ErrorWithColor");
	}
	else if (AutoAssemblySummary.WarningCount > 0)
	{
		Info.Image = FAppStyle::GetBrush("Icons.WarningWithColor");
	}
	else
	{
		Info.Image = FAppStyle::GetBrush("Icons.SuccessWithColor");
	}
	FSlateNotificationManager::Get().AddNotification(Info);
}

void UNWorldAssemblyEditorSubsystem::ScheduleNextAutoAssembly()
{
	// QuickAssemblyAutoAssemblyTimer is user-editable; clamp so a zero/negative value can't assert or busy-loop.
	const float Delay = FMath::Max(KINDA_SMALL_NUMBER, UNWorldAssemblyEditorUserSettings::Get()->QuickAssemblyAutoAssemblyTimer);
	GEditor->GetTimerManager()->SetTimer(AutoAssemblyTimerHandle,
		FTimerDelegate::CreateUObject(this, &UNWorldAssemblyEditorSubsystem::OnAutoAssemblyTimerElapsed),
		Delay, false);
}

void UNWorldAssemblyEditorSubsystem::OnAutoAssemblyTimerElapsed()
{
	// One-shot timer: it has fired, so the handle is spent.
	AutoAssemblyTimerHandle.Invalidate();

	// Already disengaged (e.g. a cancel landed during the wait) - nothing to summarize here.
	if (!bAutoAssemblyLoopActive)
	{
		StopAutoAssemblyLoop(/*bEmitSummary*/ false);
		return;
	}

	// Auto Assembly was toggled off during the inter-run wait - a deliberate user stop, so surface the summary.
	if (!UNWorldAssemblyEditorUserSettings::Get()->bQuickAssemblyAutoAssembly)
	{
		StopAutoAssemblyLoop(/*bEmitSummary*/ true);
		return;
	}

	// A new run can't start right now (no valid organ, in PIE, or another operation is in flight). This is an
	// environment-driven stop, not a user one, so drop the tally silently and keep the button off "cancel".
	if (!FNWorldAssemblyEditorCommands::StartQuickAssembly_CanExecute())
	{
		StopAutoAssemblyLoop(/*bEmitSummary*/ false);
		return;
	}

	FNWorldAssemblyEditorCommands::StartQuickAssembly();
}

void UNWorldAssemblyEditorSubsystem::UpdateAutoAssemblyCountdownBar()
{
	if (!bAutoAssemblyLoopActive || !AutoAssemblyTimerHandle.IsValid())
	{
		return;
	}

	// Fill the bar 0..1 as the inter-run delay elapses, so it reads as a countdown to the next run.
	const FTimerManager& TimerManager = *GEditor->GetTimerManager();
	const float Rate = TimerManager.GetTimerRate(AutoAssemblyTimerHandle);
	if (Rate > 0.0f)
	{
		const float Remaining = TimerManager.GetTimerRemaining(AutoAssemblyTimerHandle);
		FNWorldAssemblyEditorToolMenu::SetQuickAssemblyProgress(1.0f - (Remaining / Rate));
	}
}
