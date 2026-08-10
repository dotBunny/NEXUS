// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorCommands.h"

#include "LevelEditor.h"
#include "EdMode/NWorldAssemblyEdMode.h"
#include "NWorldAssemblyEditorUtils.h"
#include "NWorldAssemblyEditorQuickAssembly.h"
#include "Framework/Commands/UICommandList.h"
#include "EdMode/NWorldAssemblyEdModeCellDataRail.h"
#include "EdMode/NWorldAssemblyEdModeCellRail.h"
#include "EdMode/NWorldAssemblyEdModeJunctionRail.h"
#include "EdMode/NWorldAssemblyEdModeOrganRail.h"
#include "EdMode/NWorldAssemblyEdModeWorldRail.h"

void FNWorldAssemblyEditorCommands::RegisterCommands()
{
	const TSharedRef<FBindingContext> Context = this->AsShared();

	// The rails own their own commands, so registration goes through them. Static, and deliberately so: this runs once
	// at module startup, while a rail instance is built fresh every time the edit mode is opened.
	FNWorldAssemblyEdModeWorldRail::RegisterCommands(Context);
	FNWorldAssemblyEdModeCellRail::RegisterCommands(Context);
	FNWorldAssemblyEdModeCellDataRail::RegisterCommands(Context);
	FNWorldAssemblyEdModeJunctionRail::RegisterCommands(Context);
	FNWorldAssemblyEdModeOrganRail::RegisterCommands(Context);

	// No rail of its own — driven from the tool menu and the subsystem — so it stays a category class.
	FNWorldAssemblyEditorQuickAssembly::Register(Context);
}

TSharedRef<FUICommandList> FNWorldAssemblyEditorCommands::MakeCommandList(const TArray<FNCommandAction>& Actions)
{
	const TSharedRef<FUICommandList> CommandList = MakeShared<FUICommandList>();

	for (const FNCommandAction& Action : Actions)
	{
		if (!Action.CommandInfo.IsValid()) continue;

		CommandList->MapAction(Action.CommandInfo, FUIAction(Action.Execute, Action.CanExecute, Action.IsChecked));
	}

	return CommandList;
}

void FNWorldAssemblyEditorCommands::WorldAssemblyEdMode()
{
	GLevelEditorModeTools().ActivateMode(UNWorldAssemblyEdMode::Identifier);
}

bool FNWorldAssemblyEditorCommands::WorldAssemblyEdMode_CanShow()
{
	if (UNWorldAssemblyEdMode::IsActive()) return false;
	return FNWorldAssemblyEditorUtils::IsOrganComponentPresentInCurrentWorld() || FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld();
}
