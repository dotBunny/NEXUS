// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "EdMode/NWorldAssemblyEdModeWorldRail.h"

#include "Editor.h"
#include "NEditorStyle.h"
#include "NEditorUtils.h"
#include "NUIEditorStyle.h"
#include "EdMode/NWorldAssemblyEdMode.h"
#include "NWorldAssemblyEditorCommands.h"
#include "NWorldAssemblyEditorStyle.h"
#include "NWorldAssemblyEditorUtils.h"
#include "EdMode/NWorldAssemblyEdModePaletteCommands.h"
#include "NWorldAssemblyMinimal.h"
#include "Developer/NDebugActor.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/UICommandList.h"
#include "NWorldAssemblyEditorCellUtils.h"
#include "NWorldAssemblyEditorOrganUtils.h"
#include "NWorldAssemblyEditorTagUtils.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

TSharedPtr<FUICommandList> FNWorldAssemblyEdModeWorldRail::CategoryCommandList;

TSharedPtr<FUICommandInfo> FNWorldAssemblyEdModeWorldRail::CommandInfo_ToggleCollisionVisualizer;
TSharedPtr<FUICommandInfo> FNWorldAssemblyEdModeWorldRail::CommandInfo_AddCellActor;
TSharedPtr<FUICommandInfo> FNWorldAssemblyEdModeWorldRail::CommandInfo_AddOrganVolume;
TSharedPtr<FUICommandInfo> FNWorldAssemblyEdModeWorldRail::CommandInfo_TagCollisionIgnore;

void FNWorldAssemblyEdModeWorldRail::RegisterCommands(const TSharedRef<FBindingContext>& Context)
{
	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_ToggleCollisionVisualizer,
		"NWorldAssembly.World.ToggleCollisionVisualizer",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_World_ToggleCollisionVisualizer", "Collision Visualizer"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_World_ToggleCollisionVisualizer_Tooltip", "Creates and destroys a temporary/transient visualizer of the worlds collision geometry used during assembly."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Tool.CollisionVisualizer"),
		EUserInterfaceActionType::ToggleButton, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_AddCellActor,
		"NWorldAssembly.World.AddCellActor",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_World_AddCellActor", "Add Cell Actor"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_World_AddCellActor_Tooltip", "Create the singleton-like actor which will facilitate creating a NCell from the level it is placed in."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Rail.Cell.Icon"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_AddOrganVolume,
		"NWorldAssembly.World.AddOrganVolume",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_World_AddOrganVolume", "Add Organ Volume"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_World_AddOrganVolume_Tooltip", "Place a new Organ Volume in the current level, which bounds where an assembly operation may generate."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Rail.Organ.Icon"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_TagCollisionIgnore,
		"NWorldAssembly.World.TagCollisionIgnore",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_World_TagCollisionIgnore", "World Collision"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_World_TagCollisionIgnore_Tooltip", "Toggles the necessary tag to have the selected actors ignored in the world collision system when placing Cells during assembly."),
		FSlateIcon(FNUIEditorStyle::GetStyleSetName(), "Command.Tag"),
		EUserInterfaceActionType::Button, FInputChord());

	using FOperations = FNWorldAssemblyEditorCellUtils;
	using FOrganOperations = FNWorldAssemblyEditorOrganUtils;

	CategoryCommandList = FNWorldAssemblyEditorCommands::MakeCommandList({
		{ CommandInfo_ToggleCollisionVisualizer, FExecuteAction::CreateStatic(&ToggleCollisionVisualizer),      FCanExecuteAction::CreateStatic(&FNEditorUtils::IsNotPlayInEditor), FIsActionChecked::CreateStatic(&ToggleCollisionVisualizer_IsActionChecked) },
		{ CommandInfo_AddCellActor,              FExecuteAction::CreateStatic(&FOperations::AddActor),          FCanExecuteAction::CreateStatic(&AddCellActor_CanExecute) },
		{ CommandInfo_AddOrganVolume,            FExecuteAction::CreateStatic(&FOrganOperations::AddVolume),    FCanExecuteAction::CreateStatic(&AddOrganVolume_CanExecute) },
		{ CommandInfo_TagCollisionIgnore,        FExecuteAction::CreateStatic(&TagCollisionIgnore),             FCanExecuteAction::CreateStatic(&TagCollisionIgnore_CanExecute) },
	});
}

TSharedRef<FUICommandList> FNWorldAssemblyEdModeWorldRail::GetCommandList()
{
	return CategoryCommandList.ToSharedRef();
}

void FNWorldAssemblyEdModeWorldRail::ToggleCollisionVisualizer()
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

bool FNWorldAssemblyEdModeWorldRail::ToggleCollisionVisualizer_IsActionChecked()
{
	return UNWorldAssemblyEdMode::HasCollisionVisualizer();
}

bool FNWorldAssemblyEdModeWorldRail::AddCellActor_CanExecute()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	// One cell per level, and never in a level that assembles them: a cell is a building block an operation places,
	// while a level holding organs is the world those blocks are placed into. Making a level both is not a thing the
	// assembly pipeline can act on, so the button greys out rather than letting it be authored.
	return !UNWorldAssemblyEdMode::HasCellActor() && !FNWorldAssemblyEditorUtils::IsOrganComponentPresentInCurrentWorld();
}

bool FNWorldAssemblyEdModeWorldRail::AddOrganVolume_CanExecute()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	// The other half of AddCellActor_CanExecute's rule, which until now only refused from one side: a cell is a
	// building block an operation places, and a level holding organs is the world those blocks are placed into. That
	// makes the two mutually exclusive, so a level already authored as a cell refuses organs the same way a level
	// holding organs refuses a cell.
	//
	// No count of the level's organs, unlike the cell test — a level is meant to hold as many organ volumes as the
	// user wants, and only the cell is a singleton.
	return !UNWorldAssemblyEdMode::HasCellActor();
}

void FNWorldAssemblyEdModeWorldRail::TagCollisionIgnore()
{
	FNWorldAssemblyEditorTagUtils::ToggleTagOnSelection(
		NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore,
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEdModeWorldRail_TagCollisionIgnore_Add", "Add WorldCollisionIgnore Tags"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEdModeWorldRail_TagCollisionIgnore_Remove", "Remove WorldCollisionIgnore Tags"));
}

FSlateIcon FNWorldAssemblyEdModeWorldRail::TagCollisionIgnoreIcon()
{
	// Asks the same question ToggleTagOnSelection asks to pick its transaction, so the button cannot promise one thing
	// and do the other: any tagged actor in the selection means the next click strips the tag from all of them.
	const bool bWouldRemove = FNWorldAssemblyEditorTagUtils::IsTagOnAnySelectedActor(
		NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore);

	return FSlateIcon(FNUIEditorStyle::GetStyleSetName(), bWouldRemove ? "Command.ToggleOn" : "Command.ToggleOff");
}

bool FNWorldAssemblyEdModeWorldRail::TagCollisionIgnore_CanExecute()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	// Offered only in a level that is not itself a cell: world collision is what an assembly operation places cells
	// against, so tagging it out is a decision about the destination level, not about a cell's own geometry.
	return UNWorldAssemblyEdMode::IsActive() && !UNWorldAssemblyEdMode::HasCellActor() && FNEditorUtils::HasActorsSelected();
}

TSharedPtr<FUICommandInfo> FNWorldAssemblyEdModeWorldRail::GetCategoryCommand() const
{
	return FNWorldAssemblyEdModePaletteCommands::Get().LoadWorldPalette;
}

TSharedPtr<SWidget> FNWorldAssemblyEdModeWorldRail::CreateContent() const
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateCommandPalette(
				{ CommandInfo_ToggleCollisionVisualizer })
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			// What turns an empty level into one the Cell or Organ rail has anything to act on, which is why both are
			// world commands: the category each unlocks stays off the rail until one of these has been run.
			CreateCommandList(
				{ CommandInfo_AddCellActor, CommandInfo_AddOrganVolume })
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateGroupSeparator()
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			// Icon bound rather than taken from the command: it reports whether the tag is already on the selection,
			// which the registered icon cannot do.
			CreateCommandList(
				{ { CommandInfo_TagCollisionIgnore, TAttribute<FSlateIcon>::CreateStatic(&TagCollisionIgnoreIcon) } })
		];
}

#undef LOCTEXT_NAMESPACE
