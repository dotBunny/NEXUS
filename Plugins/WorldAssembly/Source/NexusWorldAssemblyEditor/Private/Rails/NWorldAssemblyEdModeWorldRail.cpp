// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Rails/NWorldAssemblyEdModeWorldRail.h"

#include "NWorldAssemblyEditorCommands.h"
#include "NWorldAssemblyEdModePaletteCommands.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

TSharedPtr<FUICommandInfo> FNWorldAssemblyEdModeWorldRail::GetCategoryCommand() const
{
	return FNWorldAssemblyEdModePaletteCommands::Get().LoadWorldPalette;
}

TSharedPtr<SWidget> FNWorldAssemblyEdModeWorldRail::CreateContent() const
{
	const FNWorldAssemblyEditorCommands& Commands = FNWorldAssemblyEditorCommands::Get();

	return SNew(SVerticalBox)

		// The two commands that bring something into the level. They live on this always-present category precisely
		// because the categories they belong to are hidden until the thing they create exists. Both stay mapped in
		// their own command lists, which is what they are — only the buttons live here.
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandList(
				LOCTEXT("WorldHeader_Create", "Create"),
				{ Commands.CommandInfo_CellAddActor, Commands.CommandInfo_OrganAddVolume })
		]

		// Both concern the world collision the assembly pass tests placements against — one visualizes it, the other
		// excludes actors from it.
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandList(
				LOCTEXT("WorldHeader_Collision", "Collision"),
				{ Commands.CommandInfo_WorldToggleCollisionVisualizer, Commands.CommandInfo_WorldTagCollisionIgnore })
		];
}

#undef LOCTEXT_NAMESPACE
