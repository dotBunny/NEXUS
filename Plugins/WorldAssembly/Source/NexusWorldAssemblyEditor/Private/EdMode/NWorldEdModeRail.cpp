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
#include "NWorldAssemblyEditorSubsystem.h"
#include "NWorldAssemblyEditorTagUtils.h"
#include "NWorldCollisionPreview.h"
#include "Organ/NOrganComponent.h"
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
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_World_TagCollisionIgnore_Tooltip", "Toggles the necessary tag to have the selection ignored in the world collision system when placing Cells during assembly. Acts on the selected components when any are selected, and on the selected actors otherwise."),
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
	UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	if (Subsystem == nullptr) return;

	if (Subsystem->HasCollisionVisualizer())
	{
		Subsystem->DestroyCollisionVisualizer();
		return;
	}

	if (ANDebugActor* NewVisualizer = Subsystem->CreateCollisionVisualizer(FNEditorUtils::GetCurrentWorld()))
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
	const UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	return Subsystem != nullptr && Subsystem->HasCollisionVisualizer();
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
	// Selection narrows the bake; an empty selection means the whole level. The work itself lives in
	// FNWorldAssemblyEditorUtils, shared with the cache actor's details panel, so both bake the same way.
	FNWorldAssemblyEditorUtils::CacheWorldCollision(
		FNEditorUtils::GetCurrentWorld(), FNWorldAssemblyEditorUtils::GetSelectedOrganComponents());
}

FSlateIcon FNWorldEdModeRail::CacheWorldCollisionIcon()
{
	// Asked of FNWorldCollisionPreview, like the cache actor's status row and the visualizer, so the three cannot
	// disagree about whether the level is showable. The answer is memoized and only recomputed when an edit or a bake
	// invalidates it, which is what lets an icon attribute — re-read on every paint — ask it at all.
	const bool bStale = FNWorldCollisionPreview::GetState(FNEditorUtils::GetCurrentWorld())
		!= FNWorldCollisionPreview::EState::Available;

	return FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.BakeWorldCollision", NAME_None,
		bStale ? FName("Command.BakeWorldCollision.Stale") : NAME_None);
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
	// Components when any are selected, actors otherwise. World collision is gathered per primitive, so the tag means
	// the same thing at either level — and the finer one is what a generated actor needs, since its whole output
	// hangs off a single container actor that an actor tag would take all of.
	//
	// Selecting a component leaves its owning actor selected too, so the component question has to be asked first or
	// this would never see anything but the actor.
	if (FNWorldAssemblyEditorTagUtils::HasComponentsSelected())
	{
		FNWorldAssemblyEditorTagUtils::ToggleTagOnComponentSelection(
			NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore,
			NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEdModeWorldRail_TagCollisionIgnoreComponent_Add", "Add WorldCollisionIgnore Component Tags"),
			NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEdModeWorldRail_TagCollisionIgnoreComponent_Remove", "Remove WorldCollisionIgnore Component Tags"));
		return;
	}

	FNWorldAssemblyEditorTagUtils::ToggleTagOnSelection(
		NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore,
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEdModeWorldRail_TagCollisionIgnore_Add", "Add WorldCollisionIgnore Tags"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEdModeWorldRail_TagCollisionIgnore_Remove", "Remove WorldCollisionIgnore Tags"));
}

FSlateIcon FNWorldEdModeRail::TagCollisionIgnoreIcon()
{
	// Asks the same question TagCollisionIgnore asks, on the same selection it would act on, so the button cannot
	// promise one thing and do the other: any tagged entry in the selection means the next click strips the tag from
	// all of them.
	const bool bWouldRemove = FNWorldAssemblyEditorTagUtils::HasComponentsSelected()
		? FNWorldAssemblyEditorTagUtils::IsTagOnAnySelectedComponent(NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore)
		: FNWorldAssemblyEditorTagUtils::IsTagOnAnySelectedActor(NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore);

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
			//
			// Icon bound rather than taken from the command: it badges itself while the level's collision is out of
			// date, which the registered icon cannot do.
			CreateCommandList(
				{ { CommandInfo_CacheWorldCollision, TAttribute<FSlateIcon>::CreateStatic(&CacheWorldCollisionIcon) } })
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
