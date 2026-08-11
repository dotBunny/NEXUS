// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "EdMode/NWorldAssemblyEdModeJunctionRail.h"

#include "Cell/NCellJunctionComponent.h"
#include "Editor.h"
#include "NEditorUtils.h"
#include "NUIEditorStyle.h"
#include "NWorldAssemblyEditorCommands.h"
#include "NWorldAssemblyEditorStyle.h"
#include "NWorldAssemblyEditorUtils.h"
#include "EdMode/NWorldAssemblyEdMode.h"
#include "EdMode/NWorldAssemblyEdModePaletteCommands.h"
#include "EdMode/NWorldAssemblyEdModeToolCommands.h"
#include "NWorldAssemblyRegistry.h"
#include "Selection.h"
#include "NWorldAssemblyEditorJunctionUtils.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

TSharedPtr<FUICommandList> FNWorldAssemblyEdModeJunctionRail::CategoryCommandList;

TSharedPtr<FUICommandInfo> FNWorldAssemblyEdModeJunctionRail::CommandInfo_AddComponent;
TSharedPtr<FUICommandInfo> FNWorldAssemblyEdModeJunctionRail::CommandInfo_CollectComponents;

/**
 * @return Every junction component the current editor selection covers.
 * @note The component selection alone, unlike an organ's. A junction is selected as a component whichever way it is
 *       reached, so there is no actor to fall back to.
 */
static TArray<UNCellJunctionComponent*> GetSelectedJunctions()
{
	TArray<UNCellJunctionComponent*> Selected;

	USelection* SelectedComponents = GEditor->GetSelectedComponents();
	if (SelectedComponents == nullptr) return Selected;

	for (FSelectionIterator It(*SelectedComponents); It; ++It)
	{
		if (UNCellJunctionComponent* Junction = Cast<UNCellJunctionComponent>(*It))
		{
			Selected.AddUnique(Junction);
		}
	}
	return Selected;
}

void FNWorldAssemblyEdModeJunctionRail::RegisterCommands(const TSharedRef<FBindingContext>& Context)
{
	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_AddComponent,
		"NWorldAssembly.NCellJunction.AddComponent",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellJunction_AddComponent", "Add Component"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellJunction_AddComponent_Tooltip", "Add a NCellJunction component to current actor."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Rail.Junction.Icon"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_CollectComponents,
		"NWorldAssembly.NCellJunction.CollectComponents",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellJunction_CollectComponents", "Collect All"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellJunction_CollectComponents_Tooltip", "Collects all Junctions and move them to the selected Actor, maintaining their world transforms."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Rail.Junction.CollectJunctions"),
		EUserInterfaceActionType::Button, FInputChord());

	using FOperations = FNWorldAssemblyEditorJunctionUtils;

	CategoryCommandList = FNWorldAssemblyEditorCommands::MakeCommandList({
		{ CommandInfo_AddComponent,      FExecuteAction::CreateStatic(&FOperations::AddComponent),      FCanExecuteAction::CreateStatic(&FNEditorUtils::HasActorsSelected) },
		{ CommandInfo_CollectComponents, FExecuteAction::CreateStatic(&FOperations::CollectComponents), FCanExecuteAction::CreateStatic(&CanEditCellJunction) },
	});
}

TSharedRef<FUICommandList> FNWorldAssemblyEdModeJunctionRail::GetCommandList()
{
	return CategoryCommandList.ToSharedRef();
}

bool FNWorldAssemblyEdModeJunctionRail::CanEditCellJunction()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	// A junction is a component of a cell, so there has to be one — the old junction dropdown was hidden outright
	// without it, and these commands would otherwise attach junctions to a level that is not a cell at all.
	return UNWorldAssemblyEdMode::HasCellActor() && FNEditorUtils::HasActorsSelected();
}

TSharedPtr<FUICommandInfo> FNWorldAssemblyEdModeJunctionRail::GetCategoryCommand() const
{
	return FNWorldAssemblyEdModePaletteCommands::Get().LoadJunctionPalette;
}

TAttribute<bool> FNWorldAssemblyEdModeJunctionRail::GetAvailable() const
{
	// Junctions hang off cells, so the category is relevant exactly when the level has a cell to hang them off —
	// the same test the Cell category uses, and level contents rather than the focused actor for the same reason.
	return TAttribute<bool>::CreateStatic(&FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld);
}

/** @return A combo box naming the selected junction, and listing the level's others to switch to. */
static TSharedRef<SWidget> CreateJunctionPicker()
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
					FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "ClassIcon.NCellJunctionComponent"),
					FUIAction(
						FExecuteAction::CreateStatic(&FNWorldAssemblyEditorJunctionUtils::SelectComponent, Junction),
						FCanExecuteAction(),
						// Rebuilds the selection per entry while the menu is open. Bounded by the junctions in the
						// level and only while the list is down, which keeps one walk of the selection as the single
						// answer both the entries and the button below are drawn from.
						FIsActionChecked::CreateLambda([Junction]() { return GetSelectedJunctions().Contains(Junction); })),
					NAME_None,
					// Check rather than RadioButton: the selection this reports back can hold several junctions at
					// once, and a column of filled radios says the opposite of what a radio means. Picking one still
					// replaces the selection — the mark is what is selected, not what a click will do.
					EUserInterfaceActionType::Check);
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
				const TArray<UNCellJunctionComponent*> Selected = GetSelectedJunctions();

				// Named only when there is one name to give. Listing several would outgrow the button, and a count
				// would leave the user working out whether it meant junctions selected or junctions in the level.
				if (Selected.Num() > 1)
				{
					return LOCTEXT("JunctionPicker_Multiple", "Multiple Selected");
				}

				if (Selected.Num() == 1)
				{
					return FText::FromString(Selected[0]->GetName());
				}

				return FNWorldAssemblyRegistry::GetCellJunctionsComponentsFromLevel(FNEditorUtils::GetCurrentLevel()).IsEmpty()
					? LOCTEXT("JunctionPicker_Empty", "No Junctions")
					: LOCTEXT("JunctionPicker_None", "Select Junction");
			})
		];
}

TSharedPtr<SWidget> FNWorldAssemblyEdModeJunctionRail::CreateContent() const
{
	const FNWorldAssemblyEdModeToolCommands& ToolCommands = FNWorldAssemblyEdModeToolCommands::Get();

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateCommandPalette(
				{ ToolCommands.BeginJunctionPlacementTool })
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateCommandList(
				{ CommandInfo_AddComponent, CommandInfo_CollectComponents })
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateGroupSeparator()
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.Padding(4.0f, 4.0f, 4.0f, 2.0f)
		[
			CreateJunctionPicker()
		];
}

#undef LOCTEXT_NAMESPACE
