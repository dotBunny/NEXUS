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
	// Drop the tracked ticket for the quick-assembly operation we were following so the toolbar button can leave
	// its "operation running" state. This is the natural-completion path only; a cancelled operation routes
	// through OnOperationDestroyed instead and must never re-arm the loop.
	if (Operation->GetTicket() == FNWorldAssemblyEditorToolMenu::GetQuickAssemblyOperationTicket())
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
			StopAutoAssemblyLoop();
		}
	}

	for (ANCellProxy* Proxy : TaskGraphContext->CreatedProxies)
	{
		KnownProxies.Add(Proxy);
	}

	// Make our own map to the created proxies tied to the operation ticket
	ProxyMap.Add(Operation->GetTicket(), TArray<TObjectPtr<ANCellProxy>>(TaskGraphContext->CreatedProxies));
	KnownOperations.Remove(Operation);

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
	bAutoAssemblyLoopActive = true;
}

void UNWorldAssemblyEditorSubsystem::StopAutoAssemblyLoop()
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

	// Stop cleanly if the loop was disengaged, Auto Assembly was toggled off, or a new run can't start right now
	// (no valid organ, in PIE, or another operation is in flight). This keeps the button from sticking on "cancel".
	if (!bAutoAssemblyLoopActive ||
		!UNWorldAssemblyEditorUserSettings::Get()->bQuickAssemblyAutoAssembly ||
		!FNWorldAssemblyEditorCommands::StartQuickAssembly_CanExecute())
	{
		StopAutoAssemblyLoop();
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
