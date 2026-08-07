// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Rails/NWorldAssemblyEdModeOrganRail.h"

#include "Editor.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyEditorStyle.h"
#include "NWorldAssemblyEditorUtils.h"
#include "NWorldAssemblyEdModePaletteCommands.h"
#include "NWorldAssemblyRegistry.h"
#include "Commands/NWorldAssemblyEditorOrganCommands.h"
#include "Operations/NWorldAssemblyEditorOrganOperations.h"
#include "Organ/NOrganComponent.h"
#include "Organ/NOrganVolume.h"
#include "Selection.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/SNAssemblyOperations.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

/**
 * @return The organ component in the current editor selection, or nullptr when none is selected.
 * @note Two selections have to be consulted, unlike a junction's. Picking from the organ list selects the
 *       component, but an organ lives on an ANOrganVolume — so selecting one in the outliner or the viewport
 *       selects the actor instead, and only the actor.
 */
static UNOrganComponent* GetSelectedOrgan()
{
	if (USelection* SelectedComponents = GEditor->GetSelectedComponents())
	{
		for (FSelectionIterator It(*SelectedComponents); It; ++It)
		{
			if (UNOrganComponent* Organ = Cast<UNOrganComponent>(*It))
			{
				return Organ;
			}
		}
	}

	if (USelection* SelectedActors = GEditor->GetSelectedActors())
	{
		for (FSelectionIterator It(*SelectedActors); It; ++It)
		{
			if (const ANOrganVolume* Volume = Cast<ANOrganVolume>(*It))
			{
				return Volume->GetOrganComponent();
			}
		}
	}
	return nullptr;
}

TSharedPtr<FUICommandInfo> FNWorldAssemblyEdModeOrganRail::GetCategoryCommand() const
{
	return FNWorldAssemblyEdModePaletteCommands::Get().LoadOrganPalette;
}

TAttribute<bool> FNWorldAssemblyEdModeOrganRail::GetEnabled() const
{
	return TAttribute<bool>::CreateStatic(&FNWorldAssemblyEditorUtils::IsOrganComponentPresentInCurrentWorld);
}

TSharedPtr<SWidget> FNWorldAssemblyEdModeOrganRail::CreateHeader() const
{
	return SNew(SComboButton)
		.ToolTipText(LOCTEXT("OrganPicker_ToolTip", "Select an Organ in the current level."))
		.OnGetMenuContent_Lambda([]() -> TSharedRef<SWidget>
		{
			FMenuBuilder MenuBuilder(true, nullptr);
			MenuBuilder.SetSearchable(false);

			for (UNOrganComponent* Organ : FNWorldAssemblyRegistry::GetOrganComponentsFromLevel(FNEditorUtils::GetCurrentLevel()))
			{
				const FText OrganName = FText::FromString(Organ->GetDebugLabel());
				MenuBuilder.AddMenuEntry(
					OrganName,
					FText::Format(LOCTEXT("OrganPicker_Select", "Select {0}"), OrganName),
					FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NOrganComponent"),
					FUIAction(
						FExecuteAction::CreateStatic(&FNWorldAssemblyEditorOrganOperations::SelectComponent, Organ),
						FCanExecuteAction(),
						FIsActionChecked::CreateLambda([Organ]() { return GetSelectedOrgan() == Organ; })),
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
				// Reports the selection rather than a count, the same way the junction picker does: choosing from this
				// list selects an organ in the level, so the button reads back what is selected — including selections
				// made in the viewport or outliner. GetDebugLabel to match how the entries below are named.
				if (const UNOrganComponent* Selected = GetSelectedOrgan())
				{
					return FText::FromString(Selected->GetDebugLabel());
				}

				return FNWorldAssemblyRegistry::GetOrganComponentsFromLevel(FNEditorUtils::GetCurrentLevel()).IsEmpty()
					? LOCTEXT("OrganPicker_Empty", "No Organs")
					: LOCTEXT("OrganPicker_None", "Select Organ");
			})
		];
}

TSharedPtr<SWidget> FNWorldAssemblyEdModeOrganRail::CreateContent() const
{
	const FNWorldAssemblyEditorOrganCommands& OrganCommands = FNWorldAssemblyEditorOrganCommands::Get();

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandGrid(
				LOCTEXT("OrganHeader_SelectedOrgan", "Selected Organ"),
				{
					OrganCommands.CommandInfo_GenerateProxies,
					OrganCommands.CommandInfo_ClearProxies,
					OrganCommands.CommandInfo_CreateLevelInstances,
					OrganCommands.CommandInfo_UnloadLevelInstances,
				})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledCommandList(
				LOCTEXT("OrganHeader_World", "World"),
				{
					OrganCommands.CommandInfo_GenerateAllProxies,
					OrganCommands.CommandInfo_ClearAllProxies,
					OrganCommands.CommandInfo_CreateAllLevelInstances,
					OrganCommands.CommandInfo_UnloadAllLevelInstances,
				})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitledContent(
				LOCTEXT("OrganHeader_Operations", "Operations"),
				SNew(SNAssemblyOperations))
		];
}

#undef LOCTEXT_NAMESPACE
