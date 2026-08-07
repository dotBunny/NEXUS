// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Commands/NWorldAssemblyEditorQuickAssemblyCommands.h"

#include "NEditorUtils.h"
#include "NWorldAssemblyEditorSubsystem.h"
#include "NWorldAssemblyEditorUserSettings.h"
#include "NWorldAssemblyRegistry.h"
#include "Assembly/NAssemblyOperation.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/UICommandList.h"
#include "NEditorUtils.h"
#include "Organ/NOrganComponent.h"

/** The organ Quick Assembly targets. Weak, so a deleted organ falls back to the level's first rather than dangling. */
static TWeakObjectPtr<UNOrganComponent> SelectedOrgan = nullptr;

/** Ticket of the operation this category started, or -1 when nothing is tracked. */
static int32 TrackedOperationTicket = -1;

/** Toolbar progress fill (0..1). Unset when no operation is in flight, which hides the bar. */
static TOptional<float> ToolbarProgress;

FNWorldAssemblyEditorQuickAssemblyCommands& FNWorldAssemblyEditorQuickAssemblyCommands::Get()
{
	static FNWorldAssemblyEditorQuickAssemblyCommands Instance;
	return Instance;
}

void FNWorldAssemblyEditorQuickAssemblyCommands::Register(const TSharedRef<FBindingContext>& Context)
{
	FNWorldAssemblyEditorQuickAssemblyCommands& Commands = Get();

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_ToggleLoadInstances,
		"NWorldAssembly.QuickAssembly.ToggleLoadInstances",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_QuickAssembly_ToggleLoadInstances", "Load Cell Instances"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_QuickAssembly_ToggleLoadInstances_Tooltip", "Load the Cell Instances themselves from the generated ANCellProxy(s)."),
		FSlateIcon(), EUserInterfaceActionType::Check, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_ToggleAutoAssembly,
		"NWorldAssembly.QuickAssembly.ToggleAutoAssembly",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_QuickAssembly_ToggleAutoAssembly", "Enabled"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_QuickAssembly_ToggleAutoAssembly_Tooltip", "Should the selected organ continue to run assembly operations after completion, until stopped."),
		FSlateIcon(), EUserInterfaceActionType::Check, FInputChord());

	Commands.CommandList = MakeShared<FUICommandList>();

	Commands.CommandList->MapAction(Commands.CommandInfo_ToggleLoadInstances,
		FExecuteAction::CreateStatic(&ToggleLoadInstances),
		FCanExecuteAction(),
		FIsActionChecked::CreateStatic(&ToggleLoadInstances_IsActionChecked));

	Commands.CommandList->MapAction(Commands.CommandInfo_ToggleAutoAssembly,
		FExecuteAction::CreateStatic(&ToggleAutoAssembly),
		FCanExecuteAction(),
		FIsActionChecked::CreateStatic(&ToggleAutoAssembly_IsActionChecked));
}

void FNWorldAssemblyEditorQuickAssemblyCommands::ButtonClicked()
{
	// "Active" spans both a running operation and the wait between auto-assembly runs, so a click during the
	// inter-run gap cancels the loop rather than kicking off a second, overlapping operation.
	if (IsActive())
	{
		Cancel();
	}
	else
	{
		Start();
	}
}

bool FNWorldAssemblyEditorQuickAssemblyCommands::Button_CanExecute()
{
	// While the loop is active (operation running or waiting between auto-runs) the button must stay enabled so the
	// user can cancel it; otherwise fall back to the start preconditions (which require that no operation is running).
	if (IsActive()) return true;
	return Start_CanExecute();
}

void FNWorldAssemblyEditorQuickAssemblyCommands::Start()
{
	UNOrganComponent* Component = GetSelectedOrgan();
	if (Component == nullptr) return;

	UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	Subsystem->ClearGenerated(Component->GetAndResetLastOperationTicket());

	const UNWorldAssemblyEditorUserSettings* UserSettings = UNWorldAssemblyEditorUserSettings::Get();

	FNAssemblyOperationSettings EditorSettings = FNAssemblyOperationSettings::GetDefaultEditorSettings();
	EditorSettings.bCreateLevelInstances = UserSettings->bQuickAssemblyLoadLevelInstances;

	UNAssemblyOperation* Operation = UNAssemblyOperation::CreateInstance(Component, EditorSettings);
	SetOperationTicket(Operation->GetTicket());

	// Drive the toolbar progress bar from the operation's combined task + sub-channel progress, so it keeps
	// moving during long-running tasks. The delegate lives on the operation, so it auto-detaches when the
	// operation is destroyed - no manual unbind needed.
	SetProgress(0.0f); // Show an empty bar immediately.
	Operation->OnPercentageChanged.AddDynamic(Subsystem, &UNWorldAssemblyEditorSubsystem::OnQuickAssemblyProgressChanged);

	Subsystem->StartOperation(Operation);

	// With Auto Assembly enabled, engage the loop so the button stays in its cancel state through the inter-run
	// waits; the subsystem re-arms the timer each time the operation finishes (this same function is re-entered
	// from the timer, so settings are re-read live every run). Guard on IsRunning so a no-op StartOperation
	// (e.g. invalid context) doesn't leave the loop engaged with nothing scheduled.
	if (Operation->IsRunning() && UserSettings->bQuickAssemblyAutoAssembly)
	{
		Subsystem->BeginAutoAssemblyLoop();
	}
}

bool FNWorldAssemblyEditorQuickAssemblyCommands::Start_CanExecute()
{
	return	HasValidOrgan() &&
			FNEditorUtils::IsNotPlayInEditor() &&
			!FNWorldAssemblyRegistry::HasOperations();
}

void FNWorldAssemblyEditorQuickAssemblyCommands::Cancel()
{
	// Disengage the auto-assembly loop first so the pending operation's teardown can't re-arm the inter-run timer.
	// When cancelled during the inter-run gap (no live operation) this also clears the toolbar countdown bar. A
	// cancel is a deliberate user stop, so surface the accumulated pass/warn/fail summary for any completed runs.
	UNWorldAssemblyEditorSubsystem::Get()->StopAutoAssemblyLoop(/*bEmitSummary*/ true);

	if (UNAssemblyOperation* Operation = GetTrackedOperation();
		Operation != nullptr && Operation->IsRunning())
	{
		// Synchronous: cancels the task graph and tears the operation down, which routes through the subsystem's
		// OnOperationDestroyed to clear the progress bar and reset the tracked ticket.
		Operation->Cancel();
	}
}

void FNWorldAssemblyEditorQuickAssemblyCommands::ToggleLoadInstances()
{
	UNWorldAssemblyEditorUserSettings* Settings = UNWorldAssemblyEditorUserSettings::GetMutable();
	Settings->bQuickAssemblyLoadLevelInstances = !Settings->bQuickAssemblyLoadLevelInstances;
	Settings->SaveConfig();
}

bool FNWorldAssemblyEditorQuickAssemblyCommands::ToggleLoadInstances_IsActionChecked()
{
	return UNWorldAssemblyEditorUserSettings::Get()->bQuickAssemblyLoadLevelInstances;
}

void FNWorldAssemblyEditorQuickAssemblyCommands::ToggleAutoAssembly()
{
	UNWorldAssemblyEditorUserSettings* Settings = UNWorldAssemblyEditorUserSettings::GetMutable();
	Settings->bQuickAssemblyAutoAssembly = !Settings->bQuickAssemblyAutoAssembly;
	Settings->SaveConfig();
}

bool FNWorldAssemblyEditorQuickAssemblyCommands::ToggleAutoAssembly_IsActionChecked()
{
	return UNWorldAssemblyEditorUserSettings::Get()->bQuickAssemblyAutoAssembly;
}

UNOrganComponent* FNWorldAssemblyEditorQuickAssemblyCommands::GetSelectedOrgan()
{
	// Nothing selected (or the prior selection went stale): default to the first Organ in the level, if any.
	if (!SelectedOrgan.IsValid())
	{
		TArray<UNOrganComponent*> OrganComponents = FNWorldAssemblyRegistry::GetOrganComponentsFromLevel(FNEditorUtils::GetCurrentLevel());
		if (OrganComponents.Num() > 0)
		{
			SetSelectedOrgan(OrganComponents[0]);
		}
	}

	if (!SelectedOrgan.IsValid()) return nullptr;
	return SelectedOrgan.Get();
}

void FNWorldAssemblyEditorQuickAssemblyCommands::SetSelectedOrgan(UNOrganComponent* OrganComponent)
{
	// Authoritative lock: don't let the Quick Assembly target change while a loop it kicked off is active
	// (operation running or waiting between auto-runs).
	if (IsActive()) return;
	SelectedOrgan = OrganComponent;
}

bool FNWorldAssemblyEditorQuickAssemblyCommands::HasValidOrgan()
{
	// Route through the getter so the same first-option fallback applies everywhere.
	return GetSelectedOrgan() != nullptr;
}

void FNWorldAssemblyEditorQuickAssemblyCommands::SetProgress(const float InProgress)
{
	ToolbarProgress = FMath::Clamp(InProgress, 0.0f, 1.0f);
}

void FNWorldAssemblyEditorQuickAssemblyCommands::ClearProgress()
{
	ToolbarProgress.Reset();
}

TOptional<float> FNWorldAssemblyEditorQuickAssemblyCommands::GetProgress()
{
	return ToolbarProgress;
}

void FNWorldAssemblyEditorQuickAssemblyCommands::SetOperationTicket(const int32 Ticket)
{
	TrackedOperationTicket = Ticket;
}

int32 FNWorldAssemblyEditorQuickAssemblyCommands::GetOperationTicket()
{
	return TrackedOperationTicket;
}

UNAssemblyOperation* FNWorldAssemblyEditorQuickAssemblyCommands::GetTrackedOperation()
{
	// A negative ticket means we are not tracking an operation (the subsystem resets it on finish/destroy).
	if (TrackedOperationTicket < 0) return nullptr;

	for (UNAssemblyOperation* Operation : FNWorldAssemblyRegistry::GetOperations())
	{
		if (Operation != nullptr && Operation->GetTicket() == TrackedOperationTicket)
		{
			return Operation;
		}
	}
	return nullptr;
}

bool FNWorldAssemblyEditorQuickAssemblyCommands::IsOperationRunning()
{
	const UNAssemblyOperation* Operation = GetTrackedOperation();
	return Operation != nullptr && Operation->IsRunning();
}

bool FNWorldAssemblyEditorQuickAssemblyCommands::IsActive()
{
	if (IsOperationRunning()) return true;

	// The loop also counts as "active" while it waits between auto-assembly runs, so the button stays in its
	// cancel state (and the organ selection stays locked) across the inter-run gap.
	const UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	return Subsystem != nullptr && Subsystem->IsAutoAssemblyLoopActive();
}
