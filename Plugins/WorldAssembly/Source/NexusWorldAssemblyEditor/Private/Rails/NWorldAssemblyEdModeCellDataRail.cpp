// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Rails/NWorldAssemblyEdModeCellDataRail.h"

#include "NWorldAssemblyEditorUtils.h"
#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblyEdModePaletteCommands.h"
#include "NWorldAssemblyEdModeToolCommands.h"

#include "Commands/NWorldAssemblyEditorCellDataCommands.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

TSharedPtr<FUICommandInfo> FNWorldAssemblyEdModeCellDataRail::GetCategoryCommand() const
{
	return FNWorldAssemblyEdModePaletteCommands::Get().LoadCellDataPalette;
}

TAttribute<bool> FNWorldAssemblyEdModeCellDataRail::GetAvailable() const
{
	// Level contents rather than the focused actor. The category is worth showing whenever the level has a cell to
	// author, whether or not one is selected this instant; the buttons inside it are what go dead when nothing is
	// focused.
	return TAttribute<bool>::CreateStatic(&FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld);
}

TSharedPtr<SWidget> FNWorldAssemblyEdModeCellDataRail::CreateContent() const
{
	const FNWorldAssemblyEditorCellDataCommands& Cell = FNWorldAssemblyEditorCellDataCommands::Get();

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandList(
				LOCTEXT("CellHeader_Actions", "Actions"),
				{
					Cell.CommandInfo_RemoveActor,
					Cell.CommandInfo_CaptureThumbnail,
				})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandGrid(
				LOCTEXT("CellHeader_CellData", "Cell Data"),
				{
					Cell.CommandInfo_SaveCell,
					Cell.CommandInfo_ResetCell,

				})
		];
}

#undef LOCTEXT_NAMESPACE
