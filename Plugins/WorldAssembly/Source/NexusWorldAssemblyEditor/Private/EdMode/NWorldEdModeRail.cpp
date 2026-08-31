// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "EdMode/NWorldEdModeRail.h"

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
#include "NWorldAssemblySettings.h"
#include "NWorldCollisionBaker.h"
#include "NWorldCollisionCacheSave.h"
#include "Organ/NOrganComponent.h"
#include "ScopedTransaction.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

TSharedPtr<FUICommandList> FNWorldEdModeRail::CategoryCommandList;

TSharedPtr<FUICommandInfo> FNWorldEdModeRail::CommandInfo_ToggleCollisionVisualizer;
TSharedPtr<FUICommandInfo> FNWorldEdModeRail::CommandInfo_CacheWorldCollision;
TSharedPtr<FUICommandInfo> FNWorldEdModeRail::CommandInfo_AddCellActor;
TSharedPtr<FUICommandInfo> FNWorldEdModeRail::CommandInfo_AddOrganVolume;
TSharedPtr<FUICommandInfo> FNWorldEdModeRail::CommandInfo_TagCollisionIgnore;

void FNWorldEdModeRail::RegisterCommands(const TSharedRef<FBindingContext>& Context)
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

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_CacheWorldCollision,
		"NWorldAssembly.World.CacheWorldCollision",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_World_CacheWorldCollision", "Cache World Collision"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_World_CacheWorldCollision_Tooltip", "Bakes the world collision geometry the level's organs assemble against, so a run reads it instead of gathering the level again. Acts on the selected organs, or on every organ in the level when none are selected."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.BakeWorldCollision"),
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
		{ CommandInfo_CacheWorldCollision,       FExecuteAction::CreateStatic(&CacheWorldCollision),            FCanExecuteAction::CreateStatic(&CacheWorldCollision_CanExecute) },
		{ CommandInfo_TagCollisionIgnore,        FExecuteAction::CreateStatic(&TagCollisionIgnore),             FCanExecuteAction::CreateStatic(&TagCollisionIgnore_CanExecute) },
	});
}

TSharedRef<FUICommandList> FNWorldEdModeRail::GetCommandList()
{
	return CategoryCommandList.ToSharedRef();
}

void FNWorldEdModeRail::ToggleCollisionVisualizer()
{
	if (UNWorldAssemblyEdMode::HasCollisionVisualizer())
	{
		UNWorldAssemblyEdMode::DestroyCollisionVisualizer();
		return;
	}

	if (const TObjectPtr<ANDebugActor> NewVisualizer = UNWorldAssemblyEdMode::CreateCollisionVisualizer(FNEditorUtils::GetCurrentWorld()))
	{
		// Exclusively, rather than added to whatever the user had selected when they hit the button. The visualizer is
		// one actor standing in for the whole level's collision, and it is built from the actors most likely to be
		// selected at that moment — so an additive selection leaves the gizmo and the details panel on geometry the
		// user has just replaced their view of, and dragging it moves the source out from under the merge.
		//
		// Notified, unlike the select this replaced: with the deselect above passing false, nothing else would call
		// NoteSelectionChange and the details panel would sit on the outgoing selection.
		GEditor->SelectNone(false, true);
		GEditor->SelectActor(NewVisualizer, true, true, true, true);
	}
}

bool FNWorldEdModeRail::ToggleCollisionVisualizer_IsActionChecked()
{
	return UNWorldAssemblyEdMode::HasCollisionVisualizer();
}

bool FNWorldEdModeRail::AddCellActor_CanExecute()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	// One cell per level, and never in a level that assembles them: a cell is a building block an operation places,
	// while a level holding organs is the world those blocks are placed into. Making a level both is not a thing the
	// assembly pipeline can act on, so the button greys out rather than letting it be authored.
	return !UNWorldAssemblyEdMode::HasCellActor() && !FNWorldAssemblyEditorUtils::IsOrganComponentPresentInCurrentWorld();
}

bool FNWorldEdModeRail::AddOrganVolume_CanExecute()
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

void FNWorldEdModeRail::CacheWorldCollision()
{
	UWorld* World = FNEditorUtils::GetCurrentWorld();
	if (World == nullptr) return;

	const FNWorldAssemblyWorldCollisionSettings& Settings = UNWorldAssemblySettings::Get()->WorldCollisionSettings;

	// Selection narrows the bake; an empty selection means the whole level. The bake writes to the organ components
	// and to the level's cache actor, so it is transacted like any other authoring action.
	const TArray<UNOrganComponent*> SelectedOrgans = FNWorldAssemblyEditorUtils::GetSelectedOrganComponents();

	const FScopedTransaction Transaction(
		LOCTEXT("FNWorldAssemblyEdModeWorldRail_CacheWorldCollision", "Cache World Collision"));

	const FNWorldCollisionBaker::FBakeResult Result = SelectedOrgans.IsEmpty()
		? FNWorldCollisionBaker::BakeWorld(World, Settings, true)
		: FNWorldCollisionBaker::BakeOrgans(World, SelectedOrgans, Settings, true);

	// Every organ has just been fingerprinted against the live world, so the save-time pass has nothing left to find.
	// Only sound for a whole-level bake that ran to completion — a selected-organ bake leaves the rest of the level
	// unexamined, and a cancelled one leaves the organs it never reached unexamined too.
	if (SelectedOrgans.IsEmpty() && !Result.bCancelled)
	{
		FNWorldCollisionCacheSave::MarkClean(World);
	}

	// Reported rather than silent: the bake's whole value is that it moves work off the assembly, and the only way to
	// see it happened is to say so. A run that changed nothing is worth saying too — it means the level is unchanged.
	UE_LOG(LogNexusWorldAssembly, Log, TEXT("World collision cache: baked %d organ(s)%s%s."),
		Result.OrgansBaked,
		Result.bChanged ? TEXT("") : TEXT(" (nothing changed)"),
		Result.bCancelled ? TEXT(" - cancelled before finishing") : TEXT(""));
}

bool FNWorldEdModeRail::CacheWorldCollision_CanExecute()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	// Offered on the same terms as the collision-ignore tag: this is a decision about a destination level, and a
	// level authored as a cell is not one. Without an organ there is nothing whose collision could be baked.
	return UNWorldAssemblyEdMode::IsActive() && !UNWorldAssemblyEdMode::HasCellActor() &&
		FNWorldAssemblyEditorUtils::IsOrganComponentPresentInCurrentWorld();
}

void FNWorldEdModeRail::TagCollisionIgnore()
{
	FNWorldAssemblyEditorTagUtils::ToggleTagOnSelection(
		NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore,
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEdModeWorldRail_TagCollisionIgnore_Add", "Add WorldCollisionIgnore Tags"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEdModeWorldRail_TagCollisionIgnore_Remove", "Remove WorldCollisionIgnore Tags"));
}

FSlateIcon FNWorldEdModeRail::TagCollisionIgnoreIcon()
{
	// Asks the same question ToggleTagOnSelection asks to pick its transaction, so the button cannot promise one thing
	// and do the other: any tagged actor in the selection means the next click strips the tag from all of them.
	const bool bWouldRemove = FNWorldAssemblyEditorTagUtils::IsTagOnAnySelectedActor(
		NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore);

	return FSlateIcon(FNUIEditorStyle::GetStyleSetName(), bWouldRemove ? "Command.ToggleOn" : "Command.ToggleOff");
}

bool FNWorldEdModeRail::TagCollisionIgnore_CanExecute()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	// Offered only in a level that is not itself a cell: world collision is what an assembly operation places cells
	// against, so tagging it out is a decision about the destination level, not about a cell's own geometry.
	return UNWorldAssemblyEdMode::IsActive() && !UNWorldAssemblyEdMode::HasCellActor() && FNEditorUtils::HasActorsSelected();
}

TSharedPtr<FUICommandInfo> FNWorldEdModeRail::GetCategoryCommand() const
{
	return FNWorldAssemblyEdModePaletteCommands::Get().LoadWorldPalette;
}

TSharedPtr<SWidget> FNWorldEdModeRail::CreateContent() const
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
			// Sits with the visualizer rather than with the Add commands: both act on the level's world collision,
			// one showing it and one storing it.
			CreateCommandList(
				{ CommandInfo_CacheWorldCollision })
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
			CreateGroupSeparator(LOCTEXT("WorldSelectedActorSeparator", "SELECTED ACTOR"))
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
