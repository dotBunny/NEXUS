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

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandPalette(
				LOCTEXT("WorldHeader_Visualizers", "Visualizers"),
				{ Commands.CommandInfo_WorldToggleCollisionVisualizer })
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandList(
				LOCTEXT("WorldHeader_Create", "Create"),
				{ Commands.CommandInfo_CellAddActor, Commands.CommandInfo_OrganAddVolume })
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandList(
				LOCTEXT("WorldHeader_Tagging", "Tagging"),
				{ Commands.CommandInfo_WorldTagCollisionIgnore })
		];
}

#undef LOCTEXT_NAMESPACE
