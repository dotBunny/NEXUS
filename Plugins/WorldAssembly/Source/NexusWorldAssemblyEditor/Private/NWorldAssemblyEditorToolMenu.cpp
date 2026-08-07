// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorToolMenu.h"

#include "FileHelpers.h"
#include "Misc/PackageName.h"
#include "Selection.h"
#include "NCoreEditorMinimal.h"
#include "Styling/AppStyle.h"        // FAppStyle, used by N_DYNAMIC_SEPARATOR
#include "Styling/ToolBarStyle.h"    // FToolBarStyle, used by N_DYNAMIC_SEPARATOR
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"

#include "Cell/NCellJunctionComponent.h"
#include "NWorldAssemblyRegistry.h"
#include "NEditorStyle.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyEditorCommands.h"
#include "Commands/NWorldAssemblyEditorQuickAssemblyCommands.h"
#include "NWorldAssemblyEditorModule.h"
#include "NWorldAssemblyEditorSubsystem.h"
#include "NWorldAssemblyEditorUserSettings.h"
#include "NWorldAssemblyEditorUtils.h"
#include "NWorldAssemblyEdMode.h"
#include "Commandlets/NUpdateCellDataCommandlet.h"
#include "Macros/NEditorToolsMacros.h"

const FName FNWorldAssemblyEditorToolMenu::MenuSectionGlobal = FName("NEXUS_WorldAssemblyGlobal");

void FNWorldAssemblyEditorToolMenu::AddMenuEntries()
{
	// Level Tools
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser))
	{
		// Safety that it is already added for some reason
		const FToolMenuSection* ExistingSection = Menu->FindSection(MenuSectionGlobal);
		if (ExistingSection != nullptr) return;

		// The Organ commands are deliberately not appended to the level editor's global action list any more. They now
		// live on the edit mode's toolkit command list, so their chords fire only while the mode is open — which is
		// also the only time the palette that hosts them is on screen.

		// Everything that survives here is reachable without entering the edit mode: the button that enters it, and
		// the Quick Assembly cluster. The rest moved to the toolkit panel's rail palettes.
		FToolMenuSection& NexusGlobalSection = Menu->AddSection(MenuSectionGlobal);
		NexusGlobalSection.Visibility =  TAttribute<EVisibility>::CreateLambda([]()
		{
			if (FNWorldAssemblyEditorUtils::IsOrganComponentPresentInCurrentWorld() || FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld())
			{
				return EVisibility::Visible;
			}
			return EVisibility::Collapsed;
		});

		// Add a button that if a NCellActor/Pin is selected and were not in the ToolMode it will show and clicking switches mode
		NexusGlobalSection.AddEntry(N_DYNAMIC_SEPARATOR("NexusSection_QuickAssemblySeparator", FNWorldAssemblyEditorCommands::WorldAssemblyEdMode_CanShow() ? EVisibility::Visible : EVisibility::Collapsed, FText::GetEmpty()));

		const FToolMenuEntry NWorldAssemblyEdMode_Button = FToolMenuEntry::InitToolBarButton(
					"NWorldAssemblyEdMode_Button",
					FUIAction(
						FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCommands::WorldAssemblyEdMode),
						FCanExecuteAction::CreateStatic(&UNWorldAssemblyEdMode::IsNotActive),
						FIsActionChecked(),
						FIsActionButtonVisible::CreateStatic(&FNWorldAssemblyEditorCommands::WorldAssemblyEdMode_CanShow)),
						NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_Button", "Switch To WorldAssembly Editor Mode"),
						NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_Button_Tooltip", "Switch the current editor mode to the WorldAssembly Editor Mode, which enables specific tools for working with NCells, etc."),
						FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Icon.WorldAssembly"));
		NexusGlobalSection.AddEntry(NWorldAssemblyEdMode_Button);

		// Quick Assembly Section
		FToolMenuEntry QuickAssemblyComboBox = FToolMenuEntry::InitWidget(
			"NWorldAssembly_QuickAssembly",
			CreateQuickAssemblyComboBox(), // This function returns our Slate combo box
			FText::FromString("Quick Assembly"),
			true // Should it be vertically aligned neatly in the toolbar?
		);
		NexusGlobalSection.AddEntry(QuickAssemblyComboBox);

		// Toggles between starting a Quick Assembly operation and cancelling the one it started. The icon, label and
		// tooltip all key off FNWorldAssemblyEditorQuickAssemblyCommands::IsActive() so they stay in sync across both
		// a running operation and the wait between auto-assembly runs.
		FToolMenuEntry QuickAssemblyButton = FToolMenuEntry::InitToolBarButton(
					"NWorldAssemblyEdMode_QuickAssemblyButton",
					FUIAction(
						FExecuteAction::CreateStatic(&FNWorldAssemblyEditorQuickAssemblyCommands::ButtonClicked),
						FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorQuickAssemblyCommands::Button_CanExecute),
						FIsActionChecked(),
						// Hide the button when the Quick Assembly section is hidden, i.e. the world has no Organ components.
						FIsActionButtonVisible::CreateStatic(&FNWorldAssemblyEditorToolMenu::ShowOrganDropdown)),
						TAttribute<FText>::CreateLambda([]()
						{
							return FNWorldAssemblyEditorQuickAssemblyCommands::IsActive()
								? NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_CancelQuickAssemblyButton", "Cancel World Assembly Operation")
								: NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_QuickAssemblyButton", "Start World Assembly Operation");
						}),
						TAttribute<FText>::CreateLambda([]()
						{
							return FNWorldAssemblyEditorQuickAssemblyCommands::IsActive()
								? NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_CancelQuickAssemblyButton_Tooltip", "Cancels the running World Assembly Operation for the selected Organ.")
								: NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_QuickAssemblyButton_Tooltip", "Starts a World Assembly Operation for the selected Organ, creating the NCellLevelInstances and loading their content.");
						}),
						TAttribute<FSlateIcon>::Create(
							TAttribute<FSlateIcon>::FGetter::CreateStatic(
						&FNWorldAssemblyEditorStyle::QuickAssemblyOperationIcon)));

		QuickAssemblyButton.StyleNameOverride = "Toolbar.BackplateLeft";
		NexusGlobalSection.AddEntry(QuickAssemblyButton);

		// Quick Assembly Quick Options for Quick People
		FToolMenuEntry QuickAssemblyOptionsButton = FToolMenuEntry::InitComboButton(
			"NWorldAssemblyEdMode_QuickAssemblyOptions",
			FUIAction(
				FExecuteAction(),
				FCanExecuteAction(),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&FNWorldAssemblyEditorToolMenu::ShowOrganDropdown)),
				FOnGetContent::CreateLambda([]()
				{
					const FNWorldAssemblyEditorQuickAssemblyCommands& QuickAssembly = FNWorldAssemblyEditorQuickAssemblyCommands::Get();

					FMenuBuilder MenuBuilder(true, QuickAssembly.CommandList);
					MenuBuilder.SetSearchable(false); // Life's too short to search this menu.

					MenuBuilder.BeginSection("NWorldAssemblyEdMode_QuickAssemblyOptions_CellBehavior", NSLOCTEXT("NexusWorldAssemblyEditor", "QuickAssemblyOptions_CellProxy", "Cell Proxy"));
					MenuBuilder.AddMenuEntry(QuickAssembly.CommandInfo_ToggleLoadInstances);
					MenuBuilder.EndSection();

					MenuBuilder.BeginSection("NWorldAssemblyEdMode_QuickAssemblyOptions_AutoAssembly", NSLOCTEXT("NexusWorldAssemblyEditor", "QuickAssemblyOptions_AutoAssembly", "Auto Assembly"));
					MenuBuilder.AddMenuEntry(QuickAssembly.CommandInfo_ToggleAutoAssembly);
					MenuBuilder.AddWidget(CreateQuickAssemblyAutoAssemblyTimerWidget(),
						NSLOCTEXT("NexusWorldAssemblyEditor", "Command_QuickAssembly_AutoAssemblyTimer", "Gap Timer"));
					MenuBuilder.EndSection();

					// Floor the menu width so the section headers/timer don't collapse to a cramped popup.
					return SNew(SBox)
						.MinDesiredWidth(250.0f)
						[
							MenuBuilder.MakeWidget()
						];
				}),
			NSLOCTEXT("NexusWorldAssemblyEditor", "NOrganExtensions_Label", "Organ"),
			NSLOCTEXT("NexusWorldAssemblyEditor", "NOrganExtensions_ToolTip", "Making procedural content easier since 2017.")
		);
		QuickAssemblyOptionsButton.StyleNameOverride = "Toolbar.BackplateRightCombo";
		NexusGlobalSection.AddEntry(QuickAssemblyOptionsButton);

	}

	// Tools/Commandlets Menu
	auto UpdateCellDataMenuEntry = FNMenuEntry();
	UpdateCellDataMenuEntry.Section = TEXT("Commandlets");
	UpdateCellDataMenuEntry.Identifier = "UpdateCellData";
	UpdateCellDataMenuEntry.DisplayName = NSLOCTEXT("NexusWorldAssemblyEditor","UpdateCellData", "Update Cell Data");
	UpdateCellDataMenuEntry.Tooltip = NSLOCTEXT("NexusWorldAssemblyEditor","UpdateCellData_Tooltip", "Finds and opens all levels associated to NCells, resaving their associated NCell data, correcting any drift."),
	UpdateCellDataMenuEntry.Icon = FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "ClassIcon.NCell");
	UpdateCellDataMenuEntry.Execute = FExecuteAction::CreateLambda([]()
	{
		// The commandlet swaps the active map as it loads each Cell world, discarding any unsaved work.
		// Prompt the user to save first; abort the whole operation if they cancel (Don't Save still proceeds).
		constexpr bool bPromptUserToSave = true;
		constexpr bool bSaveMapPackages = true;
		constexpr bool bSaveContentPackages = true;
		constexpr bool bFastSave = false;
		constexpr bool bNotifyNoPackagesSaved = false;
		constexpr bool bCanBeDeclined = true;
		if (!FEditorFileUtils::SaveDirtyPackages(bPromptUserToSave, bSaveMapPackages, bSaveContentPackages,
			bFastSave, bNotifyNoPackagesSaved, bCanBeDeclined))
		{
			return;
		}

		// Capture the active map so we can restore it after the commandlet loads each Cell world in turn.
		// Best-effort only: skip restoration when there is no map open, or it is transient (e.g. /Temp/Untitled).
		const UWorld* CurrentWorld = FNEditorUtils::GetCurrentWorld();
		const UPackage* CurrentPackage = CurrentWorld != nullptr ? CurrentWorld->GetPackage() : nullptr;
		const FString CurrentWorldPath = (CurrentPackage != nullptr && FPackageName::DoesPackageExist(CurrentPackage->GetName()))
			? CurrentPackage->GetName() : FString();

		UNUpdateCellDataCommandlet::Execute();

		if (!CurrentWorldPath.IsEmpty())
		{
			FEditorFileUtils::LoadMap(CurrentWorldPath);
		}
	});
	UpdateCellDataMenuEntry.CanExecute = FCanExecuteAction::CreateStatic(FNEditorUtils::IsNotPlayInEditor);
	FNToolsMenu::AddMenuEntry(UpdateCellDataMenuEntry);
}

void FNWorldAssemblyEditorToolMenu::RemoveMenuEntries()
{
	// Drop the whole section rather than individual entries. This removes every entry we register (buttons,
	// dropdowns and the dynamic separators) without having to maintain a name list that can drift out of sync,
	// and it clears the section itself so AddMenuEntries' FindSection guard re-registers cleanly on reload.
	if (UToolMenus* Menu = UToolMenus::TryGet())
	{
		Menu->RemoveSection(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser, MenuSectionGlobal);
	}

	// No global action list to detach from any more: the Organ commands are bound to the edit mode's toolkit command
	// list, which the toolkit tears down with itself when the mode exits.

	// Tools/Commandlets Menu
	FNToolsMenu::RemoveMenuEntry("UpdateCellData");
}

bool FNWorldAssemblyEditorToolMenu::ShowOrganDropdown()
{
	return FNWorldAssemblyRegistry::HasOrganComponents();
}

TSharedRef<SWidget> FNWorldAssemblyEditorToolMenu::CreateQuickAssemblyComboBox()
{
	return SNew(SBox)
		.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f)) // Left, Top, Right, Bottom
		.MinDesiredWidth(150.0f)
		// Collapse the whole combo box when the Quick Assembly section is hidden (no Organ components, or disabled in settings).
		.Visibility_Lambda([]() { return ShowOrganDropdown() ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			SNew(SComboButton)
			.OnGetMenuContent_Lambda([]() -> TSharedRef<SWidget>
		{
			// Lock the selection while the Quick Assembly loop is active (operation running or waiting between
			// auto-runs): open an empty menu so the target Organ can't be changed mid-loop. Keeps the button lit.
			if (FNWorldAssemblyEditorQuickAssemblyCommands::IsActive()) return SNullWidget::NullWidget;

			// This builds the menu that drops down when you click the button
			FMenuBuilder MenuBuilder(true, nullptr);
			TArray<UNOrganComponent*> OrganComponents = FNWorldAssemblyRegistry::GetOrganComponentsFromLevel(FNEditorUtils::GetCurrentLevel());
			for (int32 i = 0; i < OrganComponents.Num(); i++)
			{
				UNOrganComponent* OrganComponent = OrganComponents[i];
				FText OrganName = FText::FromString(OrganComponent->GetDebugLabel());
				MenuBuilder.AddMenuEntry(
					OrganName,
					FText::Format(NSLOCTEXT("NexusWorldAssemblyEditor", "SelectOrganForQuickAssembly", "Select {0} for Quick Assembly"), OrganName),
					FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NOrganComponent"),
					FUIAction(FExecuteAction::CreateLambda([OrganComponent]() {
						FNWorldAssemblyEditorQuickAssemblyCommands::SetSelectedOrgan(OrganComponent);
					}))
				);
			}
			return MenuBuilder.MakeWidget();
		})
			.ButtonContent()
			[
				SNew(SOverlay)

				// Progress bar drawn behind the content - only visible while an operation is running
				//  m .FillColorAndOpacity()
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SProgressBar)
					.Percent_Lambda([]() { return FNWorldAssemblyEditorQuickAssemblyCommands::GetProgress(); })
					.Visibility_Lambda([]()
					{
						return FNWorldAssemblyEditorQuickAssemblyCommands::GetProgress().IsSet() ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
					})
				]

				// Icon + label drawn on top of the progress bar
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				[
					// This is the label displayed on the toolbar button itself
					SNew(SHorizontalBox)

					// Organ icon - only shown when a valid Organ is selected
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 4.0f, 0.0f)
					[
						SNew(SImage)
						.Image_Lambda([]() -> const FSlateBrush*
						{
							// While the loop is active, swap the Organ icon for a padlock to signal the selection is locked.
							if (FNWorldAssemblyEditorQuickAssemblyCommands::IsActive())
							{
								return FSlateIcon(FNEditorStyle::GetStyleSetName(), "Lock.Desaturated").GetIcon();
							}
							return FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NOrganComponent").GetIcon();
						})
						.Visibility_Lambda([]()
						{
							return FNWorldAssemblyEditorQuickAssemblyCommands::GetSelectedOrgan() != nullptr ? EVisibility::Visible : EVisibility::Collapsed;
						})
					]

					// Selected Organ label (or placeholder text)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text_Lambda([]()
						{
							if (const UNOrganComponent* Organ = FNWorldAssemblyEditorQuickAssemblyCommands::GetSelectedOrgan())
							{
								return FText::FromString(Organ->GetDebugLabel());
							}
							return FText::FromString("Select Organ");
						})
					]
				]
			]
		];
}

TSharedRef<SWidget> FNWorldAssemblyEditorToolMenu::CreateQuickAssemblyAutoAssemblyTimerWidget()
{
	return SNew(SBox)
		.MinDesiredWidth(72.0f)
		.HAlign(HAlign_Right)
		.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
		[
			SNew(SNumericEntryBox<float>)
			.AllowSpin(true)
			.MinValue(2.0f)         // Negative delays make no sense; the scheduler also clamps to a tiny positive minimum.
			.MinSliderValue(2.0f)
			.MaxSliderValue(180.0f)  // Slider range only; larger values can still be typed.
			.Delta(0.1f)
			.MinDesiredValueWidth(48.0f)
			.ToolTipText(NSLOCTEXT("NexusWorldAssemblyEditor", "Command_QuickAssembly_AutoAssemblyTimer_Tooltip", "Seconds to wait after an Auto Assembly run completes before starting the next one. Applied live to the next wait."))
			// Only meaningful while Auto Assembly is enabled, so grey it out otherwise.
			.IsEnabled_Lambda([]() { return UNWorldAssemblyEditorUserSettings::Get()->bQuickAssemblyAutoAssembly; })
			.Value_Lambda([]() -> TOptional<float>
			{
				return UNWorldAssemblyEditorUserSettings::Get()->QuickAssemblyAutoAssemblyTimer;
			})
			// Update in-memory while scrubbing so the live loop and the field stay in sync; persist on commit.
			.OnValueChanged_Lambda([](float NewValue)
			{
				UNWorldAssemblyEditorUserSettings::GetMutable()->QuickAssemblyAutoAssemblyTimer = FMath::Max(0.0f, NewValue);
			})
			.OnValueCommitted_Lambda([](float NewValue, ETextCommit::Type)
			{
				UNWorldAssemblyEditorUserSettings* Settings = UNWorldAssemblyEditorUserSettings::GetMutable();
				Settings->QuickAssemblyAutoAssemblyTimer = FMath::Max(0.0f, NewValue);
				Settings->SaveConfig();
			})
		];
}
