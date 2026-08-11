// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "EdMode/NOrganEdModeRail.h"

#include "Editor.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyEditorCommands.h"
#include "NWorldAssemblyEditorStyle.h"
#include "NWorldAssemblyEditorUtils.h"
#include "EdMode/NWorldAssemblyEdModePaletteCommands.h"
#include "NWorldAssemblyRegistry.h"
#include "NWorldAssemblyEditorOrganUtils.h"
#include "Organ/NOrganComponent.h"
#include "Organ/NOrganVolume.h"
#include "Selection.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/SNWorldAssemblyOperations.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "NexusWorldAssemblyEditor"

TSharedPtr<FUICommandList> FNOrganEdModeRail::CategoryCommandList;

TSharedPtr<FUICommandInfo> FNOrganEdModeRail::CommandInfo_GenerateProxies;
TSharedPtr<FUICommandInfo> FNOrganEdModeRail::CommandInfo_GenerateAllProxies;
TSharedPtr<FUICommandInfo> FNOrganEdModeRail::CommandInfo_ClearProxies;
TSharedPtr<FUICommandInfo> FNOrganEdModeRail::CommandInfo_ClearAllProxies;
TSharedPtr<FUICommandInfo> FNOrganEdModeRail::CommandInfo_LoadLevelInstances;
TSharedPtr<FUICommandInfo> FNOrganEdModeRail::CommandInfo_CreateAllLevelInstances;
TSharedPtr<FUICommandInfo> FNOrganEdModeRail::CommandInfo_UnloadLevelInstances;
TSharedPtr<FUICommandInfo> FNOrganEdModeRail::CommandInfo_UnloadAllLevelInstances;

/**
 * @return Every organ component the current editor selection covers, without duplicates.
 * @note Two selections have to be consulted, unlike a junction's. Picking from the organ list selects the
 *       component, but an organ lives on an ANOrganVolume — so selecting one in the outliner or the viewport
 *       selects the actor instead, and only the actor. AddUnique because a selection holding both the volume and
 *       its component would otherwise report the one organ twice.
 * @note Not FNWorldAssemblyEditorUtils::GetSelectedOrganComponents, which walks the actor selection alone. That is
 *       the right answer for a command acting on what the user picked in the level, and the wrong one here — the
 *       list below selects the component, so the picker would stop reporting its own choice.
 */
static TArray<UNOrganComponent*> GetSelectedOrgans()
{
	TArray<UNOrganComponent*> Selected;

	if (USelection* SelectedComponents = GEditor->GetSelectedComponents())
	{
		for (FSelectionIterator It(*SelectedComponents); It; ++It)
		{
			if (UNOrganComponent* Organ = Cast<UNOrganComponent>(*It))
			{
				Selected.AddUnique(Organ);
			}
		}
	}

	if (USelection* SelectedActors = GEditor->GetSelectedActors())
	{
		for (FSelectionIterator It(*SelectedActors); It; ++It)
		{
			const ANOrganVolume* Volume = Cast<ANOrganVolume>(*It);
			if (Volume == nullptr) continue;

			if (UNOrganComponent* Organ = Volume->GetOrganComponent())
			{
				Selected.AddUnique(Organ);
			}
		}
	}

	return Selected;
}

void FNOrganEdModeRail::RegisterCommands(const TSharedRef<FBindingContext>& Context)
{
	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_GenerateAllProxies,
	"NWorldAssembly.NOrganComponent.GenerateAllProxies",
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_GenerateAllProxies", "Generate All Proxies"),
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_GenerateAllProxies_Tooltip", "Dispatches an NWorldAssemblyOperation via the UNWorldAssemblyEditorSubsystem to generate all UNOrganComponents in the world outputing NCellProxy actors."),
	FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Rail.Organ.GenerateAllProxies"),
	EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::Home));

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_ClearAllProxies,
	"NWorldAssembly.NOrganComponent.ClearAllProxies",
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_ClearAllProxies", "Clear All Proxies"),
	NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_ClearAllProxies_Tooltip", "Remove all generated NCellProxy actors from the world."),
	FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Rail.Organ.ClearAllProxies"),
	EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_CreateAllLevelInstances,
		"NWorldAssembly.NOrganComponent.LoadAllProxies",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_LoadAllProxies", "Load All Level Instances"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_LoadAllProxies_Tooltip", "Creates and then loads all level instances."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Rail.Organ.LoadAllLevelInstances"),
		EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::End));

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_UnloadAllLevelInstances,
		"NWorldAssembly.NOrganComponent.UnloadAllProxies",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_UnloadAllProxies", "Unload All Level Instances"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_UnloadAllProxies_Tooltip", "Unload all level instances."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Rail.Organ.UnloadAllLevelInstances"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_GenerateProxies,
		"NWorldAssembly.NOrganComponent.GenerateProxies",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_GenerateProxies", "Generate Proxies"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_GenerateProxies_Tooltip", "Dispatches an NWorldAssemblyOperation via the UNWorldAssemblyEditorSubsystem to generate the selected UNOrganComponents output NCellProxy actors."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Rail.Organ.GenerateProxies"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_ClearProxies,
			"NWorldAssembly.NOrganComponent.ClearProxies",
			NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_ClearProxies", "Clear Proxies"),
			NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_ClearProxies_Tooltip", "Remove generated NCellProxy actors from the world for the selected components operations."),
			FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Rail.Organ.ClearProxies"),
			EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_LoadLevelInstances,
		"NWorldAssembly.NOrganComponent.LoadLevelInstances",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_LoadLevelInstances", "Load Level Instances"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_LoadLevelInstances_Tooltip", "Create and/or load the level instance from the selected proxies."),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Rail.Organ.LoadLevelInstances"),
		EUserInterfaceActionType::Button, FInputChord());

	FUICommandInfo::MakeCommandInfo(Context, CommandInfo_UnloadLevelInstances,
		"NWorldAssembly.NOrganComponent.UnloadLevelInstances",
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_UnloadLevelInstances", "Unload Level Instances"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NOrganComponent_UnloadLevelInstances_Tooltip", "Unload the level instances from the selected proxies"),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Rail.Organ.UnloadLevelInstances"),
		EUserInterfaceActionType::Button, FInputChord());

	using FOperations = FNWorldAssemblyEditorOrganUtils;
	using FUtils = FNWorldAssemblyEditorUtils;
	CategoryCommandList = FNWorldAssemblyEditorCommands::MakeCommandList({
		{ CommandInfo_GenerateProxies,         FExecuteAction::CreateStatic(&FOperations::GenerateProxies),      FCanExecuteAction::CreateStatic(&FUtils::CanGenerateSelectedOrgan) },
		{ CommandInfo_GenerateAllProxies,      FExecuteAction::CreateStatic(&FOperations::GenerateAllProxies),   FCanExecuteAction::CreateStatic(&FUtils::CanGenerateAllOrgans) },
		{ CommandInfo_ClearProxies,            FExecuteAction::CreateStatic(&FOperations::ClearGenerated),       FCanExecuteAction::CreateStatic(&FUtils::HasSelectedGeneratedCellProxies) },
		{ CommandInfo_ClearAllProxies,         FExecuteAction::CreateStatic(&FOperations::ClearAllProxies),      FCanExecuteAction::CreateStatic(&FUtils::HasGeneratedCellProxies) },
		{ CommandInfo_LoadLevelInstances,    FExecuteAction::CreateStatic(&FOperations::LoadProxyLevels),      FCanExecuteAction::CreateStatic(&FUtils::HasSelectedGeneratedCellProxies) },
		{ CommandInfo_CreateAllLevelInstances, FExecuteAction::CreateStatic(&FOperations::LoadAllProxyLevels),   FCanExecuteAction::CreateStatic(&FUtils::HasGeneratedCellProxies) },
		{ CommandInfo_UnloadLevelInstances,    FExecuteAction::CreateStatic(&FOperations::UnloadProxyLevels),    FCanExecuteAction::CreateStatic(&FUtils::HasSelectedGeneratedCellProxies) },
		{ CommandInfo_UnloadAllLevelInstances, FExecuteAction::CreateStatic(&FOperations::UnloadAllProxyLevels), FCanExecuteAction::CreateStatic(&FUtils::HasGeneratedCellProxies) },
	});
}

TSharedRef<FUICommandList> FNOrganEdModeRail::GetCommandList()
{
	return CategoryCommandList.ToSharedRef();
}

TSharedPtr<FUICommandInfo> FNOrganEdModeRail::GetCategoryCommand() const
{
	return FNWorldAssemblyEdModePaletteCommands::Get().LoadOrganPalette;
}

TAttribute<bool> FNOrganEdModeRail::GetAvailable() const
{
	// Stated explicitly rather than left to the base's always-available default, which would keep the category on the
	// rail in a level with no organs.
	return TAttribute<bool>::CreateStatic(&FNWorldAssemblyEditorUtils::IsOrganComponentPresentInCurrentWorld);
}

bool FNOrganEdModeRail::ShouldAutoSelect() const
{
	// The other half of the pair the level can be: a level holding organs is one that assembles cells rather than
	// authors one, and generating from them is what the mode is there to do. Second to Cell only because a level is
	// meant to be one or the other — see AddOrganVolume_CanExecute — so the two can only both apply to data that
	// predates that rule.
	return true;
}

/** @return A combo box naming the selected organ, and listing the level's others to switch to. */
static TSharedRef<SWidget> CreateOrganPicker()
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
					FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "ClassIcon.NOrganVolume"),
					FUIAction(
						FExecuteAction::CreateStatic(&FNWorldAssemblyEditorOrganUtils::SelectComponent, Organ),
						FCanExecuteAction(),
						// Rebuilds the selection per entry while the menu is open. Bounded by the organs in the level
						// and only while the list is down, which is cheaper than it reads and keeps one walk of the
						// selection as the single answer both the entries and the button below are drawn from.
						FIsActionChecked::CreateLambda([Organ]() { return GetSelectedOrgans().Contains(Organ); })),
					NAME_None,
					// Check rather than RadioButton: the selection this reports back can hold several organs at once,
					// and a column of filled radios says the opposite of what a radio means. Picking one still
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
				// Reports the selection rather than a count, the same way the junction picker does: choosing from this
				// list selects an organ in the level, so the button reads back what is selected — including selections
				// made in the viewport or outliner. GetDebugLabel to match how the entries below are named.
				const TArray<UNOrganComponent*> Selected = GetSelectedOrgans();

				// Named only when there is one name to give. Listing several would outgrow the button, and a count
				// would leave the user working out whether it meant organs selected or organs in the level.
				if (Selected.Num() > 1)
				{
					return LOCTEXT("OrganPicker_Multiple", "Multiple Selected");
				}

				if (Selected.Num() == 1)
				{
					return FText::FromString(Selected[0]->GetDebugLabel());
				}

				return FNWorldAssemblyRegistry::GetOrganComponentsFromLevel(FNEditorUtils::GetCurrentLevel()).IsEmpty()
					? LOCTEXT("OrganPicker_Empty", "No Organs")
					: LOCTEXT("OrganPicker_None", "Select Organ");
			})
		];
}

TSharedPtr<SWidget> FNOrganEdModeRail::CreateContent() const
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateCommandList(
				{
					CommandInfo_GenerateAllProxies,
					CommandInfo_ClearAllProxies,
					CommandInfo_CreateAllLevelInstances,
					CommandInfo_UnloadAllLevelInstances,
				})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateGroupSeparator(LOCTEXT("OrganSelectedOrganSeparator", "SELECTED ORGAN"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateCommandList(
				{
					CommandInfo_GenerateProxies,
					CommandInfo_ClearProxies,
					CommandInfo_LoadLevelInstances,
					CommandInfo_UnloadLevelInstances,
				},
				CreateOrganPicker())
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateBackedContent(
				SNew(SNWorldAssemblyOperations))
		];
}

#undef LOCTEXT_NAMESPACE
