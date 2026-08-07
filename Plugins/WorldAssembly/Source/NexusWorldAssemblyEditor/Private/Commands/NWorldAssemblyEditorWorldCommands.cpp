// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Commands/NWorldAssemblyEditorWorldCommands.h"

#include "Editor.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblyEditorStyle.h"
#include "NWorldAssemblyEditorToolMenu.h"
#include "Developer/NDebugActor.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/UICommandList.h"

FNWorldAssemblyEditorWorldCommands& FNWorldAssemblyEditorWorldCommands::Get()
{
	static FNWorldAssemblyEditorWorldCommands Instance;
	return Instance;
}

void FNWorldAssemblyEditorWorldCommands::Register(const TSharedRef<FBindingContext>& Context)
{
	FNWorldAssemblyEditorWorldCommands& Commands = Get();

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_ToggleCollisionVisualizer,
		"NWorldAssembly.World.ToggleCollisionVisualizer",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_World_ToggleCollisionVisualizer", "Collision"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_World_ToggleCollisionVisualizer_Tooltip", "Creates and destroys a temporary/transient visualizer of the worlds collision geometry used during assembly."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.CreateCollisionVisualizer"),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_TagCollisionIgnore,
		"NWorldAssembly.World.TagCollisionIgnore",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_World_TagCollisionIgnore", "Ignore World Collision"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_World_TagCollisionIgnore_Tooltip", "Toggles the necessary tag to have the selected actors ignored in the world collision system when placing Cells during assembly."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.WorldCollisionIgnore_NotIgnored"),
		EUserInterfaceActionType::Button, FInputChord());

	Commands.CommandList = MakeShared<FUICommandList>();

	Commands.CommandList->MapAction(Commands.CommandInfo_ToggleCollisionVisualizer,
		FExecuteAction::CreateStatic(&ToggleCollisionVisualizer),
		FCanExecuteAction::CreateStatic(&FNEditorUtils::IsNotPlayInEditor),
		FIsActionChecked::CreateStatic(&ToggleCollisionVisualizer_IsActionChecked));

	Commands.CommandList->MapAction(Commands.CommandInfo_TagCollisionIgnore,
		FExecuteAction::CreateStatic(&TagCollisionIgnore),
		FCanExecuteAction::CreateStatic(&TagCollisionIgnore_CanExecute));
}

void FNWorldAssemblyEditorWorldCommands::ToggleCollisionVisualizer()
{
	if (UNWorldAssemblyEdMode::HasCollisionVisualizer())
	{
		UNWorldAssemblyEdMode::DestroyCollisionVisualizer();
		return;
	}

	if (const TObjectPtr<ANDebugActor> NewVisualizer = UNWorldAssemblyEdMode::CreateCollisionVisualizer(FNEditorUtils::GetCurrentWorld()))
	{
		GEditor->SelectActor(NewVisualizer, true, false, false);
	}
}

bool FNWorldAssemblyEditorWorldCommands::ToggleCollisionVisualizer_IsActionChecked()
{
	return UNWorldAssemblyEdMode::HasCollisionVisualizer();
}

void FNWorldAssemblyEditorWorldCommands::TagCollisionIgnore()
{
	FNWorldAssemblyEditorToolMenu::TagSelectedActors_WorldCollisionIgnore();
}

bool FNWorldAssemblyEditorWorldCommands::TagCollisionIgnore_CanExecute()
{
	return FNWorldAssemblyEditorToolMenu::TagSelectedActors_WorldCollisionIgnore_CanShow();
}
