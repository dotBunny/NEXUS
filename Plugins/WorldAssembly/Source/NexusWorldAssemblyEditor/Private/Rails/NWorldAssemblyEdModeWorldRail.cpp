// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Rails/NWorldAssemblyEdModeWorldRail.h"

#include "NWorldAssemblyEdModePaletteCommands.h"
#include "Commands/NWorldAssemblyEditorCellCommands.h"
#include "Commands/NWorldAssemblyEditorOrganCommands.h"
#include "Commands/NWorldAssemblyEditorWorldCommands.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

TSharedPtr<FUICommandInfo> FNWorldAssemblyEdModeWorldRail::GetCategoryCommand() const
{
	return FNWorldAssemblyEdModePaletteCommands::Get().LoadWorldPalette;
}

TSharedPtr<SWidget> FNWorldAssemblyEdModeWorldRail::CreateContent() const
{
	const FNWorldAssemblyEditorWorldCommands& World = FNWorldAssemblyEditorWorldCommands::Get();

	// The Create group is the one place a rail reaches outside its own category: adding a cell actor or an organ
	// volume is what turns an empty level into one the Cell or Organ rail has anything to act on, so both entry
	// points sit here rather than behind the category they unlock.
	const FNWorldAssemblyEditorCellCommands& Cell = FNWorldAssemblyEditorCellCommands::Get();
	const FNWorldAssemblyEditorOrganCommands& Organ = FNWorldAssemblyEditorOrganCommands::Get();

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandPalette(
				LOCTEXT("WorldHeader_Visualizers", "Visualizers"),
				{ World.CommandInfo_ToggleCollisionVisualizer })
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandList(
				LOCTEXT("WorldHeader_Create", "Create"),
				{ Cell.CommandInfo_AddActor, Organ.CommandInfo_AddVolume })
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandList(
				LOCTEXT("WorldHeader_Tagging", "Tagging"),
				{ World.CommandInfo_TagCollisionIgnore })
		];
}

#undef LOCTEXT_NAMESPACE
