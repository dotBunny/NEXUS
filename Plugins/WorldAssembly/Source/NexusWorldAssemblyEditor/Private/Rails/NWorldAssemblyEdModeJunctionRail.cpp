// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Rails/NWorldAssemblyEdModeJunctionRail.h"

#include "Cell/NCellJunctionComponent.h"
#include "Editor.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyEditorStyle.h"
#include "NWorldAssemblyEdMode.h"
#include "NWorldAssemblyEdModePaletteCommands.h"
#include "NWorldAssemblyEdModeToolCommands.h"
#include "NWorldAssemblyRegistry.h"
#include "Selection.h"
#include "Commands/NWorldAssemblyEditorJunctionCommands.h"
#include "Operations/NWorldAssemblyEditorJunctionOperations.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

/** @return The junction component in the current editor selection, or nullptr when none is selected. */
static UNCellJunctionComponent* GetSelectedJunction()
{
	USelection* SelectedComponents = GEditor->GetSelectedComponents();
	if (SelectedComponents == nullptr) return nullptr;

	for (FSelectionIterator It(*SelectedComponents); It; ++It)
	{
		if (UNCellJunctionComponent* Junction = Cast<UNCellJunctionComponent>(*It))
		{
			return Junction;
		}
	}
	return nullptr;
}

TSharedPtr<FUICommandInfo> FNWorldAssemblyEdModeJunctionRail::GetCategoryCommand() const
{
	return FNWorldAssemblyEdModePaletteCommands::Get().LoadJunctionPalette;
}

TAttribute<bool> FNWorldAssemblyEdModeJunctionRail::GetEnabled() const
{
	return TAttribute<bool>::CreateLambda(&UNWorldAssemblyEdMode::HasCellActor);
}

TSharedPtr<SWidget> FNWorldAssemblyEdModeJunctionRail::CreateHeader() const
{
	return SNew(SComboButton)
		.ToolTipText(LOCTEXT("JunctionPicker_ToolTip", "Select a Junction in the current level."))
		.OnGetMenuContent_Lambda([]() -> TSharedRef<SWidget>
		{
			FMenuBuilder MenuBuilder(true, nullptr);
			MenuBuilder.SetSearchable(false);

			for (UNCellJunctionComponent* Junction : FNWorldAssemblyRegistry::GetCellJunctionsComponentsFromLevel(FNEditorUtils::GetCurrentLevel()))
			{
				const FText JunctionName = FText::FromString(Junction->GetJunctionName());
				MenuBuilder.AddMenuEntry(
					JunctionName,
					FText::Format(LOCTEXT("JunctionPicker_Select", "Select {0}"), JunctionName),
					FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.SelectNCellJunctionComponent"),
					FUIAction(
						FExecuteAction::CreateStatic(&FNWorldAssemblyEditorJunctionOperations::SelectComponent, Junction),
						FCanExecuteAction(),
						FIsActionChecked::CreateLambda([Junction]() { return GetSelectedJunction() == Junction; })),
					NAME_None,
					EUserInterfaceActionType::RadioButton);
			}

			return MenuBuilder.MakeWidget();
		})
		.ButtonContent()
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				// Reports the selection rather than a count: picking from this list selects a junction in the level,
				// so the button reads back what is selected — including selections made in the viewport or outliner.
				//
				// The component's own name, not GetJunctionName(): that builds an "Actor > Parent > Component"
				// breadcrumb, which is useful for telling entries apart in the list below but far too long for a
				// button. The list keeps the full path; the button just names what is selected.
				if (const UNCellJunctionComponent* Selected = GetSelectedJunction())
				{
					return FText::FromString(Selected->GetName());
				}

				return FNWorldAssemblyRegistry::GetCellJunctionsComponentsFromLevel(FNEditorUtils::GetCurrentLevel()).IsEmpty()
					? LOCTEXT("JunctionPicker_Empty", "No Junctions")
					: LOCTEXT("JunctionPicker_None", "Select Junction");
			})
		];
}

TSharedPtr<SWidget> FNWorldAssemblyEdModeJunctionRail::CreateContent() const
{
	const FNWorldAssemblyEditorJunctionCommands& Junction = FNWorldAssemblyEditorJunctionCommands::Get();
	const FNWorldAssemblyEdModeToolCommands& ToolCommands = FNWorldAssemblyEdModeToolCommands::Get();

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandPalette(
				LOCTEXT("JunctionHeader_Tools", "Tools"),
				{ ToolCommands.BeginJunctionPlacementTool })
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandGrid(
				LOCTEXT("JunctionHeader_Component", "Component"),
				{ Junction.CommandInfo_AddComponent, Junction.CommandInfo_CollectComponents })
		];
}

#undef LOCTEXT_NAMESPACE
