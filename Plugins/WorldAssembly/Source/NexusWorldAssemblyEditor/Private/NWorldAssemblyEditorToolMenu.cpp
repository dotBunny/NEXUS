// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorToolMenu.h"

#include "FileHelpers.h"
#include "LevelEditor.h"
#include "Misc/PackageName.h"
#include "Selection.h"
#include "NCoreEditorMinimal.h"
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
#include "NWorldAssemblyEditorModule.h"
#include "NWorldAssemblyEditorSubsystem.h"
#include "NWorldAssemblyEditorUserSettings.h"
#include "NWorldAssemblyEditorUtils.h"
#include "NWorldAssemblyEdMode.h"
#include "Commandlets/NUpdateCellDataCommandlet.h"
#include "Macros/NEditorToolsMacros.h"
#include "Visualizers/NCellRootComponentVisualizer.h"

const FName FNWorldAssemblyEditorToolMenu::MenuSection = FName("NEXUS_WorldAssembly");

const FName FNWorldAssemblyEditorToolMenu::MenuSectionGlobal = FName("NEXUS_WorldAssemblyGlobal");
TWeakObjectPtr<UNOrganComponent> FNWorldAssemblyEditorToolMenu::QuickAssemblyOrganComponent = nullptr;
int32 FNWorldAssemblyEditorToolMenu::QuickAssemblyOperationTicket = -1;
TOptional<float> FNWorldAssemblyEditorToolMenu::QuickAssemblyProgress;

void FNWorldAssemblyEditorToolMenu::AddMenuEntries()
{
	// Level Tools
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser))
	{
		// Safety that it is already added for some reason
		const FToolMenuSection* ExistingSection = Menu->FindSection(MenuSection);
		if (ExistingSection != nullptr) return;

		// Register command lists with the level editor so input chords work globally.
		// RemoveMenuEntries detaches these again on shutdown via UnregisterGlobalActions.
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		FNWorldAssemblyEditorCommands::Get().RegisterGlobalActions(LevelEditorModule.GetGlobalLevelEditorActions());

		// Always there buttons
		FToolMenuSection& NexusGlobalSection = Menu->AddSection(MenuSectionGlobal)
		NexusGlobalSection.Visibility =  TAttribute<EVisibility>::CreateLambda([]()
		{
			if (FNWorldAssemblyEditorUtils::IsOrganComponentPresentInCurrentWorld() || FNWorldAssemblyEditorUtils::IsCellActorPresentInCurrentWorld())
			{
				return EVisibility::Visible;
			}
			return EVisibility::Collapsed;
		});

		// Add a button that if a NCellActor/Pin is selected and were not in the ToolMode it will show and clicking switches mode
		const FToolMenuEntry NWorldAssemblyEdMode_Button = FToolMenuEntry::InitToolBarButton(
					"NWorldAssemblyEdMode_Button",
					FUIAction(
						FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCommands::WorldAssemblyEdMode),
						FCanExecuteAction::CreateStatic(&FNWorldAssemblyEdMode::IsNotActive),
						FIsActionChecked(),
						FIsActionButtonVisible::CreateStatic(&FNWorldAssemblyEditorCommands::WorldAssemblyEdMode_CanShow)),
						NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_Button", "Switch To WorldAssembly Editor Mode"),
						NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_Button_Tooltip", "Switch the current editor mode to the WorldAssembly Editor Mode, which enables specific tools for working with NCells, etc."),
						FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Icon.WorldAssembly"));
		NexusGlobalSection.AddEntry(NWorldAssemblyEdMode_Button);

		// Editor Mode Dependent
		FToolMenuSection& NexusSection = Menu->AddSection(MenuSection);
		NexusSection.Visibility =  TAttribute<EVisibility>::CreateLambda([]()
		{
			if (FNWorldAssemblyEdMode::IsActive())
			{
				return EVisibility::Visible;
			}
			return EVisibility::Collapsed;
		});

		// Quick Assembly Section
		FToolMenuEntry QuickAssemblyComboBox = FToolMenuEntry::InitWidget(
			"NWorldAssembly_QuickAssembly",
			CreateQuickAssemblyComboBox(), // This function returns our Slate combo box
			FText::FromString("Quick Assembly"),
			true // Should it be vertically aligned neatly in the toolbar?
		);
		NexusSection.AddEntry(QuickAssemblyComboBox);
		// Toggles between starting a Quick Assembly operation and cancelling the one it started. The icon, label and
		// tooltip all key off FNWorldAssemblyEditorToolMenu::IsQuickAssemblyActive() so they stay in sync across both
		// a running operation and the wait between auto-assembly runs.
		FToolMenuEntry QuickAssemblyButton = FToolMenuEntry::InitToolBarButton(
					"NWorldAssemblyEdMode_QuickAssemblyButton",
					FUIAction(
						FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCommands::QuickAssemblyButtonClicked),
						FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorCommands::QuickAssemblyButton_CanExecute),
						FIsActionChecked(),
						// Hide the button when the Quick Assembly section is hidden (no Organ components, or disabled in settings).
						FIsActionButtonVisible::CreateStatic(&FNWorldAssemblyEditorToolMenu::ShowOrganDropdown)),
						TAttribute<FText>::CreateLambda([]()
						{
							return FNWorldAssemblyEditorToolMenu::IsQuickAssemblyActive()
								? NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_CancelQuickAssemblyButton", "Cancel World Assembly Operation")
								: NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_QuickAssemblyButton", "Start World Assembly Operation");
						}),
						TAttribute<FText>::CreateLambda([]()
						{
							return FNWorldAssemblyEditorToolMenu::IsQuickAssemblyActive()
								? NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_CancelQuickAssemblyButton_Tooltip", "Cancels the running World Assembly Operation for the selected Organ.")
								: NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_QuickAssemblyButton_Tooltip", "Starts a World Assembly Operation for the selected Organ, creating the NCellLevelInstances and loading their content.");
						}),
						TAttribute<FSlateIcon>::Create(
							TAttribute<FSlateIcon>::FGetter::CreateStatic(
						&FNWorldAssemblyEditorStyle::QuickAssemblyOperationIcon)));

		QuickAssemblyButton.StyleNameOverride = "Toolbar.BackplateLeft";
		NexusSection.AddEntry(QuickAssemblyButton);

		// Quick Assembly Quick Options for Quick People
		FToolMenuEntry QuickAssemblyOptionsButton = FToolMenuEntry::InitComboButton(
			"NWorldAssemblyEdMode_QuickAssemblyOptions",
			FUIAction(
				FExecuteAction(),
				FCanExecuteAction(),
				FIsActionChecked(),
				FIsActionButtonVisible()),
				FOnGetContent::CreateLambda([]()
				{
					FMenuBuilder MenuBuilder(true, FNWorldAssemblyEditorCommands::Get().CommandList_QuickAssembly);
					MenuBuilder.SetSearchable(false); // Life's too short to search this menu.

					MenuBuilder.BeginSection("NWorldAssemblyEdMode_QuickAssemblyOptions_CellBehavior", NSLOCTEXT("NexusWorldAssemblyEditor", "QuickAssemblyOptions_CellProxy", "Cell Proxy"));
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_QuickAssemblyToggleLoadInstances);
					MenuBuilder.EndSection();

					MenuBuilder.BeginSection("NWorldAssemblyEdMode_QuickAssemblyOptions_AutoAssembly", NSLOCTEXT("NexusWorldAssemblyEditor", "QuickAssemblyOptions_AutoAssembly", "Auto Assembly"));
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_QuickAssemblyToggleAutoAssembly);
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
		NexusSection.AddEntry(QuickAssemblyOptionsButton);

		// Actions Section - based on selection
		NexusSection.AddEntry(N_DYNAMIC_SEPARATOR("NexusSection_QuickAssemblySeparator", FNWorldAssemblyEditorToolMenu::ShowOrganDropdown() ? EVisibility::Visible : EVisibility::Collapsed, FText::GetEmpty()));

		// NOrgan Dropdown
		FToolMenuEntry NOrganDropdownMenu = FToolMenuEntry::InitComboButton(
			"NOrganExtensions_Button",
			FUIAction(
				FExecuteAction(),
				FCanExecuteAction::CreateStatic(&FNEditorUtils::IsNotPlayInEditor),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&FNWorldAssemblyEditorToolMenu::ShowOrganDropdown)),
				FOnGetContent::CreateLambda([]()
				{
					FMenuBuilder MenuBuilder(true, FNWorldAssemblyEditorCommands::Get().CommandList_Organ);
					MenuBuilder.SetSearchable(false); // Life's too short to search this menu.

					MenuBuilder.BeginSection("NOrganExtensions_SelectSection", NSLOCTEXT("NexusWorldAssemblyEditor", "NOrganExtensions_SelectSection", "Select Organ"));
					TArray<UNOrganComponent*> OrganComponents = FNWorldAssemblyRegistry::GetOrganComponentsFromLevel(FNEditorUtils::GetCurrentLevel());
					for (auto Organ : OrganComponents)
					{
						FText OrganName = FText::FromString(Organ->GetDebugLabel());

						MenuBuilder.AddMenuEntry(
							OrganName,
							FText::Format(NSLOCTEXT("NexusWorldAssemblyEditor", "NOrganExtensions_SelectNOrganComponent", "Select {0}"), OrganName),
							FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NOrganComponent"),
							FUIAction(FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCommands::OrganSelectComponent, Organ),
								FCanExecuteAction(),
								FIsActionChecked()));

					}
					MenuBuilder.EndSection();

					// Selected
					MenuBuilder.BeginSection("NOrganExtensions_SelectedActionSection", NSLOCTEXT("NexusWorldAssemblyEditor", "NOrganExtensions_SelectedActionSection", "Selected Actions"));
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_OrganGenerateProxies);
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_OrganClearProxies);
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_OrganCreateLevelInstances);
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_OrganUnloadLevelInstances);
					MenuBuilder.EndSection();

					// World
					MenuBuilder.BeginSection("NOrganExtensions_WorldActionSection", NSLOCTEXT("NexusWorldAssemblyEditor", "NOrganExtensions_WorldActionSection", "World Actions"));
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_OrganGenerateAllProxies);
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_OrganClearAllProxies);
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_OrganCreateAllLevelInstances);
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_OrganUnloadAllLevelInstances);
					MenuBuilder.EndSection();

					return MenuBuilder.MakeWidget();
				}),
			NSLOCTEXT("NexusWorldAssemblyEditor", "NOrganExtensions_Label", "Organ"),
			NSLOCTEXT("NexusWorldAssemblyEditor", "NOrganExtensions_ToolTip", "Making procedural content easier since 2017."),
			FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "ClassIcon.NOrganComponent")
		);
		NOrganDropdownMenu.StyleNameOverride = "CalloutToolbar";
		NexusSection.AddEntry(NOrganDropdownMenu);

		// Create our option Add NCellActor button that only shows in NWorldAssemblyEdMode + no present NCellActor
		const FToolMenuEntry NCellActor_AddButton = FToolMenuEntry::InitToolBarButton(
					"NCellActor_AddButton",
					FUIAction(
						FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCommands::CellAddActor),
						FCanExecuteAction::CreateStatic(&FNWorldAssemblyEdMode::HasNoCellActor),
						FIsActionChecked(),
						FIsActionButtonVisible::CreateStatic(&FNWorldAssemblyEditorCommands::CellAddActor_CanShow)),
						NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_AddActor", "Add Actor"),
						NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_AddActor_Tooltip", "Create the singleton-like actor which will facilitate creating a NCell from the level it is placed in."),
						FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.AddNCellActor"));
		NexusSection.AddEntry(NCellActor_AddButton);

		// Create our option to select the NCellActor if we are in NWorldAssemblyEdMode and there is a NCellActor
		const FToolMenuEntry NCellActor_SelectButton = FToolMenuEntry::InitToolBarButton(
					"NCellActor_SelectButton",
					FUIAction(
						FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCommands::CellSelectActor),
						FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorCommands::CellSelectActor_CanExecute),
						FIsActionChecked(),
						FIsActionButtonVisible::CreateStatic(&FNWorldAssemblyEditorCommands::CellSelectActor_CanShow)),
						NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_SelectActor", "Select Actor"),
						NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCell_SelectActor_Tooltip", "Select the NCellActor in the level."),
						FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.SelectNCellActor.Selected"));
		NexusSection.AddEntry(NCellActor_SelectButton);

		// Display EditMode toggle buttons
		FToolMenuEntry NCellActor_EditBoundsMode = FToolMenuEntry::InitToolBarButton(
					"NCellActor_EditBoundsMode",
					FUIAction(
						FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCommands::CellActorEditBoundsMode),
						FCanExecuteAction(),
						FIsActionChecked(),
						FIsActionButtonVisible::CreateStatic(&FNWorldAssemblyEditorToolMenu::ShowCellEditMode)),
						NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellActor_EditBoundsMode", "Edit Bounds"),
						NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellActor_EditBoundsMode_Tooltip", "Edit the bounds of the NCell."),
						TAttribute<FSlateIcon>::Create(
							TAttribute<FSlateIcon>::FGetter::CreateStatic(
						&FNWorldAssemblyEditorStyle::CellActorEditBoundsModeIcon)));
		NCellActor_EditBoundsMode.StyleNameOverride = "Toolbar.BackplateLeft";
		NexusSection.AddEntry(NCellActor_EditBoundsMode);

		FToolMenuEntry NCellActor_EditHullMode = FToolMenuEntry::InitToolBarButton(
			"NCellActor_EditHullMode",
			FUIAction(
				FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCommands::CellActorEditHullMode),
				FCanExecuteAction::CreateStatic(&FNWorldAssemblyEditorCommands::CellActorEditHullMode_CanExecute),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&FNWorldAssemblyEditorToolMenu::ShowCellEditMode)),
				NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellActor_EditHullMode", "Edit Hull"),
				NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellActor_EditHullMode_Tooltip", "Edit the hull vertices of the NCell (requires a tri-based hull)."),
				TAttribute<FSlateIcon>::Create(
					TAttribute<FSlateIcon>::FGetter::CreateStatic(
				&FNWorldAssemblyEditorStyle::CellActorEditHullModeIcon)));
		NCellActor_EditHullMode.StyleNameOverride = "Toolbar.BackplateCenter";
		NexusSection.AddEntry(NCellActor_EditHullMode);

		FToolMenuEntry NCellActor_EditVoxelMode = FToolMenuEntry::InitToolBarButton(
	"NCellActor_EditVoxelMode",
	FUIAction(
		FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCommands::CellActorEditVoxelMode),
		FCanExecuteAction(),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateStatic(&FNWorldAssemblyEditorToolMenu::ShowCellEditMode)),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellActor_EditVoxelMode", "Edit Voxels"),
		NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellActor_EditVoxelMode_Tooltip", "Edit the voxel points of the NCell."),
		TAttribute<FSlateIcon>::Create(
			TAttribute<FSlateIcon>::FGetter::CreateStatic(
		&FNWorldAssemblyEditorStyle::CellActorEditVoxelModeIcon)));
		NCellActor_EditVoxelMode.StyleNameOverride = "Toolbar.BackplateRight";
		NexusSection.AddEntry(NCellActor_EditVoxelMode);

		// NCell Dropdown
		FToolMenuEntry NCellDropdownMenu = FToolMenuEntry::InitComboButton(
			"NCellExtensions_Button",
			FUIAction(
				FExecuteAction(),
				FCanExecuteAction::CreateStatic(&FNEditorUtils::IsNotPlayInEditor),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&FNWorldAssemblyEditorToolMenu::ShowCellDropdown)),
				FOnGetContent::CreateLambda([]()
				{
					FMenuBuilder MenuBuilder(true, FNWorldAssemblyEditorCommands::Get().CommandList_Cell);
					MenuBuilder.SetSearchable(false); // Life's too short to search this menu.

					MenuBuilder.BeginSection("NCellExtensions_Asset", NSLOCTEXT("NexusWorldAssemblyEditor", "NCellExtensions_AssetSection", "Asset"));
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellCaptureThumbnail);
					MenuBuilder.EndSection();

					MenuBuilder.BeginSection("NCellExtensions_CalculateSection", NSLOCTEXT("NexusWorldAssemblyEditor", "NCellExtensions_CalculateSection", "Calculate"));
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellCalculateAll);
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellCalculateBounds);
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellCalculateHull);
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellCalculateVoxelData);
					MenuBuilder.EndSection();

					MenuBuilder.BeginSection("NCellExtensions_QuickSettings", NSLOCTEXT("NexusWorldAssemblyEditor", "NCellExtensions_QuickSettings", "Quick Settings"));
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellToggleBoundsCalculateOnSave);
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellToggleHullAllowNonConvex);
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellToggleHullCalculateOnSave);
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellToggleVoxelCalculateOnSave);
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellToggleVoxelData);
					MenuBuilder.EndSection();

					MenuBuilder.BeginSection("NCellExtensions_CleanupSection", NSLOCTEXT("NexusWorldAssemblyEditor", "NCellExtensions_CleanupSection", "Cleanup"));
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellResetCell);
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellSaveCell);
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellRemoveActor);
					MenuBuilder.EndSection();

					return MenuBuilder.MakeWidget();
				}),
			NSLOCTEXT("NexusWorldAssemblyEditor", "NCellExtensions_Label", "Cell"),
			NSLOCTEXT("NexusWorldAssemblyEditor", "NCellExtensions_ToolTip", "Making procedural content easier since 2017."),
			FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "ClassIcon.NCellActor")
		);
		NCellDropdownMenu.StyleNameOverride = "CalloutToolbar";
		NexusSection.AddEntry(NCellDropdownMenu);

		// NCellJunction Dropdown
		FToolMenuEntry NCellJunctionDropdownMenu = FToolMenuEntry::InitComboButton(
			"NCellJunctionExtensions_Button",
			FUIAction(
				FExecuteAction(),
				FCanExecuteAction::CreateStatic(&FNEditorUtils::IsNotPlayInEditor),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&FNWorldAssemblyEditorToolMenu::ShowCellJunctionDropdown)),
				FOnGetContent::CreateLambda([]()
				{
					FMenuBuilder MenuBuilder(true, FNWorldAssemblyEditorCommands::Get().CommandList_CellJunction);


					MenuBuilder.SetSearchable(false); // Life's too short to search this menu.
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellJunctionAddComponent);
					MenuBuilder.BeginSection("NCellJunctionExtensions_Section", NSLOCTEXT("NexusWorldAssemblyEditor", "NCellJunctionExtensions_Section", "Select Junction"));
					TArray<UNCellJunctionComponent*> Junctions = FNWorldAssemblyRegistry::GetCellJunctionsComponentsFromLevel(FNEditorUtils::GetCurrentLevel());
					for (auto Junction : Junctions)
					{
						FText JunctionName = FText::FromString(Junction->GetJunctionName());

						MenuBuilder.AddMenuEntry(
							JunctionName,
							FText::Format(NSLOCTEXT("NexusWorldAssemblyEditor", "SelectNCellJunctionComponent", "Select {0}"), JunctionName),
							FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.SelectNCellJunctionComponent"),
							FUIAction(FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCommands::CellJunctionSelectComponent, Junction),
								FCanExecuteAction(),
								FIsActionChecked()));

					}
					MenuBuilder.EndSection();

					return MenuBuilder.MakeWidget();
				}),
			NSLOCTEXT("NexusWorldAssemblyEditor", "NCellJunctionExtensions_Label", "Junction"),
			NSLOCTEXT("NexusWorldAssemblyEditor", "NCellJunctionExtensions_ToolTip", "Making procedural content easier since 2017."),
			FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "ClassIcon.NCellJunctionComponent")
		);
		NCellJunctionDropdownMenu.StyleNameOverride = "CalloutToolbar";
		NexusSection.AddEntry(NCellJunctionDropdownMenu);

		// Visualizers Section
		NexusSection.AddEntry(N_DYNAMIC_SEPARATOR("NexusSection_VisualizersSeparator", FNWorldAssemblyEdMode::IsActive() ? EVisibility::Visible : EVisibility::Collapsed, FText::GetEmpty()));

		// Toggle Drawing Voxel Data
		FToolMenuEntry NCellActor_DrawVoxelData = FToolMenuEntry::InitToolBarButton(
			"NCellActor_DrawVoxelData",
			FUIAction(
				FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCommands::CellActorToggleDrawVoxelData),
				FCanExecuteAction(),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&FNWorldAssemblyEditorToolMenu::ShowDrawVoxels)),
				NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellActor_DrawVoxelData", "Draw Voxel Data"),
				NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NCellActor_DrawVoxelData_Tooltip", "Toggle drawing calculated voxel data for the NCell."),
				TAttribute<FSlateIcon>::Create(
					TAttribute<FSlateIcon>::FGetter::CreateStatic(
				&FNWorldAssemblyEditorStyle::CellActorToggleDrawVoxelDataIcon)));
		NexusSection.AddEntry(NCellActor_DrawVoxelData);

		// Collision Visualizer
		FToolMenuEntry CollisionVisualizerEntry  = FToolMenuEntry::InitToolBarButton(
			"NWorldAssembly_ToggleCollisionVisualizer",
			FUIAction(
				FExecuteAction::CreateStatic(&CollisionVisualizerToggle),
				FCanExecuteAction::CreateStatic(&FNEditorUtils::IsNotPlayInEditor),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&FNWorldAssemblyEdMode::IsActive)),
				NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_ToggleCollisionVisualizer", "Toggle Collision Visualizer"),
				NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_ToggleCollisionVisualizer_Tooltip", "Creates and destroys a temporary/transient visualizer of the worlds collision geometry used during assembly."),
				TAttribute<FSlateIcon>::Create(
					TAttribute<FSlateIcon>::FGetter::CreateStatic(
				&FNWorldAssemblyEditorStyle::CollisionVisualizerToggleIcon)));
		NexusSection.AddEntry(CollisionVisualizerEntry);

		// Actions Section - based on selection
		NexusSection.AddEntry(N_DYNAMIC_SEPARATOR("NexusSection_ActionsSeparator", FNWorldAssemblyEdMode::IsActive() ? EVisibility::Visible : EVisibility::Collapsed, FText::GetEmpty()));

		// Ignore Actor Toggle
		FToolMenuEntry CellIgnoreToggle  = FToolMenuEntry::InitToolBarButton(
			"NWorldAssembly_CellIgnoreToggle",
			FUIAction(
				FExecuteAction::CreateStatic(&TagSelectedActors_CellIgnore),
				FCanExecuteAction(),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&TagSelectedActors_CellIgnore_CanShow)),
				NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_CellIgnoreToggle", "Ignore Actor (Cell Calculations)"),
				NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_CellIgnoreToggle_Tooltip", "Toggles the necessary tag to have the selected actors ignored when calculating the bounds/hull/etc for a Cell."),
				TAttribute<FSlateIcon>::Create(
					TAttribute<FSlateIcon>::FGetter::CreateStatic(
				&FNWorldAssemblyEditorStyle::CellIgnoreIcon)));
		NexusSection.AddEntry(CellIgnoreToggle);

		// Ignore Actor Toggle
		FToolMenuEntry WorldCollisionIgnoreToggle  = FToolMenuEntry::InitToolBarButton(
			"NWorldAssembly_WorldCollisionIgnoreToggle",
			FUIAction(
				FExecuteAction::CreateStatic(&TagSelectedActors_WorldCollisionIgnore),
				FCanExecuteAction(),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&TagSelectedActors_WorldCollisionIgnore_CanShow)),
				NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_WorldCollisionIgnoreToggle", "Ignore Actor (World Collision))"),
				NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_WorldCollisionIgnoreToggle_Tooltip", "Toggles the necessary tag to have the selected actors ignored in the world collision system when placing Cells during assembly."),
				TAttribute<FSlateIcon>::Create(
					TAttribute<FSlateIcon>::FGetter::CreateStatic(
				&FNWorldAssemblyEditorStyle::WorldCollisionIgnoreIcon)));
		NexusSection.AddEntry(WorldCollisionIgnoreToggle);

		FToolMenuEntry HullSplitEdgeEntry  = FToolMenuEntry::InitToolBarButton(
			"NWorldAssembly_Hull_SplitEdge",
			FUIAction(
				FExecuteAction::CreateStatic(&Hull_SplitEdge),
				FCanExecuteAction(),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&Hull_SplitEdge_CanShow)),
				NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_Hull_SplitEdge", "Split Hull Edge"),
				NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_Hull_SplitEdge_Tooltip", "Splits the hull edge and retriangulates the Hull."),
				FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.Hull.SplitEdge"));
		NexusSection.AddEntry(HullSplitEdgeEntry);
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
	// Drop the whole sections rather than individual entries. This removes every entry we register (buttons,
	// dropdowns and the dynamic separators) without having to maintain a name list that can drift out of sync,
	// and it clears the sections themselves so AddMenuEntries' FindSection guard re-registers cleanly on reload.
	if (UToolMenus* Menu = UToolMenus::TryGet())
	{
		Menu->RemoveSection(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser, MenuSectionGlobal);
		Menu->RemoveSection(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser, MenuSection);
	}

	// Detach the Organ command bindings appended to the level editor's global action list in AddMenuEntries.
	// Their delegates point at this module's statics, so leaving them mapped crashes when a global chord
	// (e.g. Ctrl+Shift+Home) is pressed after the module unloads.
	//
	// IsRegistered() guards the cook/commandlet path: OnPostEngineInit only calls Register() (and AddMenuEntries
	// only appends these actions) when Slate is initialized, so a headless run never creates the commands
	// singleton and FNWorldAssemblyEditorCommands::Get() would assert during shutdown.
	if (FNWorldAssemblyEditorCommands::IsRegistered() && FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		FNWorldAssemblyEditorCommands::Get().UnregisterGlobalActions(LevelEditorModule.GetGlobalLevelEditorActions());
	}

	// Tools/Commandlets Menu
	FNToolsMenu::RemoveMenuEntry("UpdateCellData");
}

bool FNWorldAssemblyEditorToolMenu::ShowCellEditMode()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;
	if (!FNWorldAssemblyEdMode::IsActive()) return false;

	return FNWorldAssemblyEditorUtils::IsCellActorSelected();
}

bool FNWorldAssemblyEditorToolMenu::ShowDrawVoxels()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;
	if (!FNWorldAssemblyEdMode::IsActive()) return false;
	if (!FNWorldAssemblyRegistry::HasRootComponents()) return false;

	return true;
}

bool FNWorldAssemblyEditorToolMenu::ShowCellDropdown()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	return FNWorldAssemblyEdMode::IsActive() && FNWorldAssemblyEdMode::HasCellActor();
}

bool FNWorldAssemblyEditorToolMenu::ShowCellJunctionDropdown()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	return FNWorldAssemblyEdMode::HasCellActor();
}

bool FNWorldAssemblyEditorToolMenu::ShowOrganDropdown()
{
	return FNWorldAssemblyRegistry::HasOrganComponents();
}

void FNWorldAssemblyEditorToolMenu::CollisionVisualizerToggle()
{
	if (FNWorldAssemblyEdMode::HasCollisionVisualizer())
	{
		FNWorldAssemblyEdMode::DestroyCollisionVisualizer();
	}
	else
	{
		const TObjectPtr<ANDebugActor> NewVisualizer = FNWorldAssemblyEdMode::CreateCollisionVisualizer(FNEditorUtils::GetCurrentWorld());
		if (NewVisualizer != nullptr)
		{
			GEditor->SelectActor(NewVisualizer, true, false, false);
		}
	}
}

void FNWorldAssemblyEditorToolMenu::TagSelectedActors_CellIgnore()
{

	if (TagSelectedActors_CellIgnore_Mode() == 0)
	{
		const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorToolMenu_TagSelectedActors_CellIgnore_Add", "Add CellIgnore Tags"));

		// ADD
		USelection* AddSelection = GEditor->GetSelectedActors();
		for (FSelectionIterator It(*AddSelection); It; ++It)
		{
			AActor* Actor = Cast<AActor>(*It);
			Actor->Modify(true);
			Actor->Tags.Add(NEXUS::WorldAssembly::ActorTags::CellIgnore);
		}
	}
	else
	{
		const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorToolMenu_TagSelectedActors_CellIgnore_Remove", "Remove CellIgnore Tags"));

		// REMOVE
		USelection* RemoveSelection = GEditor->GetSelectedActors();
		for (FSelectionIterator It(*RemoveSelection); It; ++It)
		{
			AActor* Actor = Cast<AActor>(*It);
			Actor->Modify(true);
			Actor->Tags.RemoveSwap(NEXUS::WorldAssembly::ActorTags::CellIgnore);
		}
	}
}

bool FNWorldAssemblyEditorToolMenu::TagSelectedActors_CellIgnore_CanShow()
{
	return FNWorldAssemblyEdMode::HasCellActor()
		&& FNEditorUtils::HasActorsSelected()
		&& !FNWorldAssemblyEditorUtils::IsCellActorSelected();
}

int32 FNWorldAssemblyEditorToolMenu::TagSelectedActors_CellIgnore_Mode()
{
	USelection* Selection = GEditor->GetSelectedActors();
	if (Selection == nullptr || Selection->Num() == 0)
	{
		return -1; // Disabled
	}

	int32 ActorCount = 0;
	int32 TaggedActorCount = 0;

	for (FSelectionIterator It(*Selection); It; ++It)
	{
		const AActor* Actor = Cast<AActor>(*It);
		ActorCount++;

		if (Actor->ActorHasTag(NEXUS::WorldAssembly::ActorTags::CellIgnore))
		{
			TaggedActorCount++;
		}
	}

	if (TaggedActorCount > 0)
	{
		if (ActorCount == TaggedActorCount)
		{
			return 1; // Remove
		}
		return -1; // Unknown
	}
	return 0; // Add
}


void FNWorldAssemblyEditorToolMenu::TagSelectedActors_WorldCollisionIgnore()
{
	if (TagSelectedActors_WorldIgnore_Mode() == 0)
	{
		const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorToolMenu_TagSelectedActors_WorldCollisionIgnore_Add", "Add WorldCollisionIgnore Tags"));
		// ADD
		USelection* AddSelection = GEditor->GetSelectedActors();
		for (FSelectionIterator It(*AddSelection); It; ++It)
		{
			AActor* Actor = Cast<AActor>(*It);
			Actor->Modify(true);
			Actor->Tags.Add(NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore);
		}
	}
	else
	{
		const FScopedTransaction Transaction(NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEditorToolMenu_TagSelectedActors_WorldCollisionIgnore_Remove", "Remove WorldCollisionIgnore Tags"));

		// REMOVE
		USelection* RemoveSelection = GEditor->GetSelectedActors();
		for (FSelectionIterator It(*RemoveSelection); It; ++It)
		{
			AActor* Actor = Cast<AActor>(*It);
			Actor->Modify(true);
			Actor->Tags.RemoveSwap(NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore);
		}
	}
}

bool FNWorldAssemblyEditorToolMenu::TagSelectedActors_WorldCollisionIgnore_CanShow()
{
	return FNWorldAssemblyEdMode::IsActive() && !FNWorldAssemblyEdMode::HasCellActor() && FNEditorUtils::HasActorsSelected();
}

int32 FNWorldAssemblyEditorToolMenu::TagSelectedActors_WorldIgnore_Mode()
{
	USelection* Selection = GEditor->GetSelectedActors();
	if (Selection == nullptr || Selection->Num() == 0)
	{
		return -1; // Disabled
	}

	int32 ActorCount = 0;
	int32 TaggedActorCount = 0;

	for (FSelectionIterator It(*Selection); It; ++It)
	{
		const AActor* Actor = Cast<AActor>(*It);
		ActorCount++;

		if (Actor->ActorHasTag(NEXUS::WorldAssembly::ActorTags::WorldCollisionIgnore))
		{
			TaggedActorCount++;
		}
	}

	if (TaggedActorCount > 0)
	{
		if (ActorCount == TaggedActorCount)
		{
			return 1; // Remove
		}
		return -1; // Unknown
	}
	return 0; // Add
}


void FNWorldAssemblyEditorToolMenu::Hull_SplitEdge()
{
	FNWorldAssemblyEditorModule& Module = FNWorldAssemblyEditorModule::Get();
	FIntVector2 Selection = Module.RootComponentVisualizer->GetEdgeSelection();

	ANCellActor* CellActor = FNWorldAssemblyEditorUtils::GetCellActorFromCurrentWorld();
	if (CellActor == nullptr) return;

	CellActor->SplitHullEdge(Selection.X, Selection.Y);
	Module.RootComponentVisualizer->ClearSelection();
}

bool FNWorldAssemblyEditorToolMenu::Hull_SplitEdge_CanShow()
{
	if (FNWorldAssemblyEdMode::GetCellEdMode() != FNWorldAssemblyEdMode::ENCellEdMode::Hull)
	{
		return false;
	}

	if (!FNWorldAssemblyEdMode::HasCellActor())
	{
		return false;
	}

	FNWorldAssemblyEditorModule& Module = FNWorldAssemblyEditorModule::Get();
	if (Module.RootComponentVisualizer->GetMode() == FNCellRootComponentVisualizer::ENCellEditMode::HullEdge &&
		Module.RootComponentVisualizer->HasEdgeSelected())
	{
		return true;
	}

	return false;
}

bool FNWorldAssemblyEditorToolMenu::HasValidQuickAssemblyOrgan()
{
	// Route through the getter so the same first-option fallback applies everywhere.
	return GetQuickAssemblyOrganComponent() != nullptr;
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
			if (IsQuickAssemblyActive()) return SNullWidget::NullWidget;

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
						SetSelectedQuickAssemblyOption(OrganComponent);
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
					.Percent_Lambda([]() { return QuickAssemblyProgress; })
					.Visibility_Lambda([]()
					{
						return QuickAssemblyProgress.IsSet() ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
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
							if (IsQuickAssemblyActive())
							{
								return FSlateIcon(FNEditorStyle::GetStyleSetName(), "Lock.Desaturated").GetIcon();
							}
							return FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Command.WorldAssemblyEd.NOrganComponent").GetIcon();
						})
						.Visibility_Lambda([]()
						{
							return GetQuickAssemblyOrganComponent() != nullptr ? EVisibility::Visible : EVisibility::Collapsed;
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
							if (const UNOrganComponent* Organ = GetQuickAssemblyOrganComponent())
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

void FNWorldAssemblyEditorToolMenu::SetSelectedQuickAssemblyOption(UNOrganComponent* OrganComponent)
{
	// Authoritative lock: don't let the Quick Assembly target change while a loop it kicked off is active
	// (operation running or waiting between auto-runs).
	if (IsQuickAssemblyActive()) return;
	QuickAssemblyOrganComponent = OrganComponent;
}

void FNWorldAssemblyEditorToolMenu::SetQuickAssemblyProgress(float InProgress)
{
	QuickAssemblyProgress = FMath::Clamp(InProgress, 0.0f, 1.0f);
}

void FNWorldAssemblyEditorToolMenu::ClearQuickAssemblyProgress()
{
	QuickAssemblyProgress.Reset();
}

UNOrganComponent* FNWorldAssemblyEditorToolMenu::GetQuickAssemblyOrganComponent()
{
	// Nothing selected (or the prior selection went stale): default to the first Organ in the level, if any.
	if (!QuickAssemblyOrganComponent.IsValid())
	{
		TArray<UNOrganComponent*> OrganComponents = FNWorldAssemblyRegistry::GetOrganComponentsFromLevel(FNEditorUtils::GetCurrentLevel());
		if (OrganComponents.Num() > 0)
		{
			SetSelectedQuickAssemblyOption(OrganComponents[0]);
		}
	}

	if (!QuickAssemblyOrganComponent.IsValid()) return nullptr;
	return QuickAssemblyOrganComponent.Get();
}

UNAssemblyOperation* FNWorldAssemblyEditorToolMenu::GetTrackedQuickAssemblyOperation()
{
	// A negative ticket means we are not tracking an operation (the subsystem resets it on finish/destroy).
	if (QuickAssemblyOperationTicket < 0) return nullptr;

	for (UNAssemblyOperation* Operation : FNWorldAssemblyRegistry::GetOperations())
	{
		if (Operation != nullptr && Operation->GetTicket() == QuickAssemblyOperationTicket)
		{
			return Operation;
		}
	}
	return nullptr;
}

bool FNWorldAssemblyEditorToolMenu::IsQuickAssemblyOperationRunning()
{
	const UNAssemblyOperation* Operation = GetTrackedQuickAssemblyOperation();
	return Operation != nullptr && Operation->IsRunning();
}

bool FNWorldAssemblyEditorToolMenu::IsQuickAssemblyActive()
{
	if (IsQuickAssemblyOperationRunning()) return true;

	// The loop also counts as "active" while it waits between auto-assembly runs, so the button stays in its
	// cancel state (and the organ selection stays locked) across the inter-run gap.
	const UNWorldAssemblyEditorSubsystem* Subsystem = UNWorldAssemblyEditorSubsystem::Get();
	return Subsystem != nullptr && Subsystem->IsAutoAssemblyLoopActive();
}
