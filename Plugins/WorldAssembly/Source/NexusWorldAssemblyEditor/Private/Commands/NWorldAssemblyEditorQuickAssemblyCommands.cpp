// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Commands/NWorldAssemblyEditorQuickAssemblyCommands.h"

#include "NEditorUtils.h"
#include "NWorldAssemblyEditorSubsystem.h"
#include "NWorldAssemblyEditorToolMenu.h"
#include "NWorldAssemblyEditorUserSettings.h"
#include "NWorldAssemblyRegistry.h"
#include "Assembly/NAssemblyOperation.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/UICommandList.h"
#include "Organ/NOrganComponent.h"

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
	if (FNWorldAssemblyEditorToolMenu::IsQuickAssemblyActive())
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
	if (FNWorldAssemblyEditorToolMenu::IsQuickAssemblyActive()) return true;
	return Start_CanExecute();
}

void FNWorldAssemblyEditorQuickAssemblyCommands::Start()
{
	UNOrganComponent* Component = FNWorldAssemblyEditorToolMenu::GetQuickAssemblyOrganComponent();
	if (Component == nullptr) return;

	UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	Subsystem->ClearGenerated(Component->GetAndResetLastOperationTicket());

	const UNWorldAssemblyEditorUserSettings* UserSettings = UNWorldAssemblyEditorUserSettings::Get();

	FNAssemblyOperationSettings EditorSettings = FNAssemblyOperationSettings::GetDefaultEditorSettings();
	EditorSettings.bCreateLevelInstances = UserSettings->bQuickAssemblyLoadLevelInstances;

	UNAssemblyOperation* Operation = UNAssemblyOperation::CreateInstance(Component, EditorSettings);
	FNWorldAssemblyEditorToolMenu::SetQuickAssemblyOperationTicket(Operation->GetTicket());

	// Drive the toolbar progress bar from the operation's combined task + sub-channel progress, so it keeps
	// moving during long-running tasks. The delegate lives on the operation, so it auto-detaches when the
	// operation is destroyed - no manual unbind needed.
	FNWorldAssemblyEditorToolMenu::SetQuickAssemblyProgress(0.0f); // Show an empty bar immediately.
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
	return	FNWorldAssemblyEditorToolMenu::HasValidQuickAssemblyOrgan() &&
			FNEditorUtils::IsNotPlayInEditor() &&
			!FNWorldAssemblyRegistry::HasOperations();
}

void FNWorldAssemblyEditorQuickAssemblyCommands::Cancel()
{
	// Disengage the auto-assembly loop first so the pending operation's teardown can't re-arm the inter-run timer.
	// When cancelled during the inter-run gap (no live operation) this also clears the toolbar countdown bar. A
	// cancel is a deliberate user stop, so surface the accumulated pass/warn/fail summary for any completed runs.
	UNWorldAssemblyEditorSubsystem::Get()->StopAutoAssemblyLoop(/*bEmitSummary*/ true);

	if (UNAssemblyOperation* Operation = FNWorldAssemblyEditorToolMenu::GetTrackedQuickAssemblyOperation();
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
