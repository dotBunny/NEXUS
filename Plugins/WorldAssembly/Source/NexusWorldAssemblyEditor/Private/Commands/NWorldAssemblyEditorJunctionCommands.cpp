// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Commands/NWorldAssemblyEditorJunctionCommands.h"

#include "NEditorUtils.h"
#include "NUIEditorStyle.h"
#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblyEditorStyle.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/UICommandList.h"
#include "Operations/NWorldAssemblyEditorJunctionOperations.h"

FNWorldAssemblyEditorJunctionCommands& FNWorldAssemblyEditorJunctionCommands::Get()
{
	static FNWorldAssemblyEditorJunctionCommands Instance;
	return Instance;
}

void FNWorldAssemblyEditorJunctionCommands::Register(const TSharedRef<FBindingContext>& Context)
{
	FNWorldAssemblyEditorJunctionCommands& Commands = Get();

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_AddComponent,
		"NWorldAssembly.NCellJunction.AddComponent",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellJunction_AddComponent", "Add"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellJunction_AddComponent_Tooltip", "Add a NCellJunction component to current actor."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_SelectComponent,
		"NWorldAssembly.NCellJunction.SelectComponent",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellJunction_SelectComponent", "Select Component"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellJunction_SelectComponent_Tooltip", "Select a NCellJunction in the level."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Calculate"),
		EUserInterfaceActionType::Button, FInputChord());

	// Collect Components
	FUICommandInfo::MakeCommandInfo(Context, Commands.CommandInfo_CollectComponents,
		"NWorldAssembly.NCellJunction.CollectComponents",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellJunction_CollectComponents", "Collect All"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellJunction_CollectComponents_Tooltip", "Collects all Junctions and move them to the selected Actor, maintaining their world transforms."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.Junction.CollectJunctionComponents"),
		EUserInterfaceActionType::Button, FInputChord());

	Commands.CommandList = MakeShared<FUICommandList>();

	Commands.CommandList->MapAction(Commands.CommandInfo_AddComponent,
		FExecuteAction::CreateStatic(&FNWorldAssemblyEditorJunctionOperations::AddComponent),
		FCanExecuteAction::CreateStatic(&FNEditorUtils::HasActorsSelected));

	Commands.CommandList->MapAction(Commands.CommandInfo_CollectComponents,
		FExecuteAction::CreateStatic(&FNWorldAssemblyEditorJunctionOperations::CollectComponents),
		FCanExecuteAction::CreateStatic(&CanEditCellJunction));
}

bool FNWorldAssemblyEditorJunctionCommands::CanEditCellJunction()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	// A junction is a component of a cell, so there has to be one — the old junction dropdown was hidden outright
	// without it, and these commands would otherwise attach junctions to a level that is not a cell at all.
	return UNWorldAssemblyEdMode::HasCellActor() && FNEditorUtils::HasActorsSelected();
}
