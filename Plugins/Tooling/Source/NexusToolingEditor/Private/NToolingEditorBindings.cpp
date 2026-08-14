// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NToolingEditorBindings.h"

#include "LevelEditor.h"
#include "Kismet2/DebuggerCommands.h"
#include "NQuickHighResScreenshot.h"

void FNToolingEditorBindings::RegisterCommands()
{
	// Deliberately a modified F9: bare F9 is the engine's own 1x viewport Screen Capture, so this reads as
	// the same key asking for more pixels. Alt+F9 itself is unclaimed on every platform, though the rest
	// of the column is spoken for — Ctrl+F9 by the Content Browser, Ctrl+Shift+F9 by Delete All
	// Breakpoints, and bare F9 additionally by Toggle Breakpoint in graph editors.
	FUICommandInfo::MakeCommandInfo(this->AsShared(), CommandInfo_QuickHighResScreenshot,
		"NTooling.QuickHighResScreenshot",
		NSLOCTEXT("NexusToolingEditor", "Command_QuickHighResScreenshot", "Quick High Resolution Screenshot"),
		NSLOCTEXT("NexusToolingEditor", "Command_QuickHighResScreenshot_Desc",
			"Captures the active level viewport at the multiplier set in Tooling (User), writing it straight to the editor's screenshot folder."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelViewport.HighResScreenshot"),
		EUserInterfaceActionType::Button, FInputChord(EModifierKey::Alt, EKeys::F9));
}

void FNToolingEditorBindings::MapActions()
{
	const FExecuteAction CaptureAction = FExecuteAction::CreateStatic(&FNQuickHighResScreenshot::Capture);

	// Loaded rather than fetched: the global command list is built in FLevelEditorModule::StartupModule,
	// and post-engine-init is early enough that the module may not have been asked for yet.
	const FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	LevelEditorModule.GetGlobalLevelEditorActions()->MapAction(Get().CommandInfo_QuickHighResScreenshot, CaptureAction);

	// The level editor list stops being consulted the moment PIE takes focus: key events go to the game
	// viewport, which forwards them to UEditorEngine::ProcessDebuggerCommands, and that only ever asks
	// GlobalPlayWorldActions. It is the list F8-eject and the pause/step chords live on, and mapping here
	// too is what makes a chord work in play as well as out of it.
	if (FPlayWorldCommands::GlobalPlayWorldActions.IsValid())
	{
		FPlayWorldCommands::GlobalPlayWorldActions->MapAction(Get().CommandInfo_QuickHighResScreenshot, CaptureAction);
	}
}

void FNToolingEditorBindings::UnmapActions()
{
	// Both lists outlive this module, and the actions mapped above point at a function inside it.
	if (!IsRegistered())
	{
		return;
	}

	if (const FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor")))
	{
		LevelEditorModule->GetGlobalLevelEditorActions()->UnmapAction(Get().CommandInfo_QuickHighResScreenshot);
	}

	if (FPlayWorldCommands::GlobalPlayWorldActions.IsValid())
	{
		FPlayWorldCommands::GlobalPlayWorldActions->UnmapAction(Get().CommandInfo_QuickHighResScreenshot);
	}
}
