// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorToolMenu.h"

#include "LevelEditor.h"
#include "NCoreEditorMinimal.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"

#include "Cell/NCellJunctionComponent.h"
#include "NWorldAssemblyRegistry.h"
#include "NEditorUtils.h"
#include "NWorldAssemblyEditorCommands.h"
#include "NWorldAssemblyEditorUtils.h"
#include "NWorldAssemblyEdMode.h"
#include "Macros/NEditorToolsMacros.h"

const FName FNWorldAssemblyEditorToolMenu::MenuSection = FName("NEXUS_WorldAssembly");
const FName FNWorldAssemblyEditorToolMenu::MenuSectionGlobal = FName("NEXUS_WorldAssemblyGlobal");

void FNWorldAssemblyEditorToolMenu::AddMenuEntries()
{
	// Level Tools
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser))
	{
		// Safety that it is already added for some reason
		const FToolMenuSection* ExistingSection = Menu->FindSection(MenuSection);
		if (ExistingSection != nullptr) return;
		
		// Register command lists with the level editor so input chords work globally
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.GetGlobalLevelEditorActions()->Append(FNWorldAssemblyEditorCommands::Get().CommandList_Organ.ToSharedRef());
		
		// Always there buttons
		FToolMenuSection& NexusGlobalSection = Menu->AddSection(MenuSectionGlobal);
		
		// Add a button that if a NCellActor/Pin is selected and were not in the ToolMode it will show and clicking switches mode
		const FToolMenuEntry NWorldAssemblyEdMode_Button = FToolMenuEntry::InitToolBarButton(
					"NWorldAssemblyEdMode_Button",
					FUIAction(
						FExecuteAction::CreateStatic(&FNWorldAssemblyEditorCommands::WorldAssemblyEdMode),
						FCanExecuteAction::CreateStatic(&FNWorldAssemblyEdMode::IsNotActive),
						FIsActionChecked(),
						FIsActionButtonVisible::CreateStatic(&FNWorldAssemblyEditorCommands::WorldAssemblyEdMode_CanShow)),
						NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_Button", "Switch To NWorldAssembly Editor Mode"),
						NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_Button", "Switch the current editor mode to the NWorldAssembly Editor Mode, which enables specific tools for working with NCells, etc."),
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
		
		// NOrgan Dropdown
		FToolMenuEntry NOrganDropdownMenu = FToolMenuEntry::InitComboButton(
			"NOrganExtensions_Button",
			FUIAction(
				FExecuteAction(),
				FCanExecuteAction::CreateStatic(&FNEditorUtils::IsNotPlayInEditor), // TODO: can we make this run in playmode?
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
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellToggleHullCalculateOnSave);
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellToggleVoxelCalculateOnSave);
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellToggleVoxelData);
					MenuBuilder.EndSection();
					
					MenuBuilder.BeginSection("NCellExtensions_CleanupSection", NSLOCTEXT("NexusWorldAssemblyEditor", "NCellExtensions_CleanupSection", "Cleanup"));
					MenuBuilder.AddMenuEntry(FNWorldAssemblyEditorCommands::Get().CommandInfo_CellResetCell);
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
		NexusSection.AddEntry(N_IMPLEMENT_DYNAMIC_SEPARATOR("NexusSection_VisualizersSeparator", FNWorldAssemblyEdMode::IsActive() ? EVisibility::Visible : EVisibility::Collapsed, FText::GetEmpty()));
		
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
				FCanExecuteAction(),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&FNWorldAssemblyEdMode::IsActive)),
				NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_ToggleCollisionVisualizer", "Toggle Collision Visualizer"),
				NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_ToggleCollisionVisualizer_Tooltip", "Creates and destroys a temporary/transient visualizer of the worlds collision geometry used during assembly."),
				TAttribute<FSlateIcon>::Create(
					TAttribute<FSlateIcon>::FGetter::CreateStatic(
				&FNWorldAssemblyEditorStyle::CollisionVisualizerToggleIcon)));
		NexusSection.AddEntry(CollisionVisualizerEntry);
		
		// Actions Section - based on selection
		NexusSection.AddEntry(N_IMPLEMENT_DYNAMIC_SEPARATOR("NexusSection_ActionsSeparator", FNWorldAssemblyEdMode::IsActive() ? EVisibility::Visible : EVisibility::Collapsed, FText::GetEmpty()));
		
		// Ignore Actor Toggle
		FToolMenuEntry IgnoreActorToggle  = FToolMenuEntry::InitToolBarButton(
			"NWorldAssembly_IgnoreActorToggle",
			FUIAction(
				FExecuteAction::CreateStatic(&IgnoreSelectedActorsToggle),
				FCanExecuteAction(),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&IgnoreSelectedActorsToggle_CanShow)),
				NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_IgnoreActorToggle", "Ignore Actor Toggle"),
				NSLOCTEXT("NexusWorldAssemblyEditor", "Command_NWorldAssemblyEdMode_IgnoreActorToggle_Tooltip", "Toggles the necessary tag to have the selected actors ignored when calculating the bounds/hull/etc for a NCell."),
				TAttribute<FSlateIcon>::Create(
					TAttribute<FSlateIcon>::FGetter::CreateStatic(
				&FNWorldAssemblyEditorStyle::IgnoreActorToggleIcon)));
		NexusSection.AddEntry(IgnoreActorToggle);
		
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
}

void FNWorldAssemblyEditorToolMenu::RemoveMenuEntries()
{
	UToolMenus* Menu = UToolMenus::TryGet();
	if (Menu)
	{
		Menu->RemoveEntry(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser, MenuSectionGlobal, "NWorldAssemblyEdMode_Button");
		
		Menu->RemoveEntry(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser, MenuSection, "NOrganExtensions_Button");

		Menu->RemoveEntry(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser, MenuSection, "NCellActor_AddButton");
		Menu->RemoveEntry(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser, MenuSection, "NCellActor_SelectButton");
		Menu->RemoveEntry(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser, MenuSection, "NCellActor_EditBoundsMode");
		Menu->RemoveEntry(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser, MenuSection, "NCellActor_EditHullMode");
		Menu->RemoveEntry(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser, MenuSection, "NCellActor_EditVoxelMode");
		Menu->RemoveEntry(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser, MenuSection, "NCellExtensions_Button");
		Menu->RemoveEntry(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser, MenuSection, "NCellJunctionExtensions_Button");
		Menu->RemoveEntry(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser, MenuSection, "NCellActor_DrawVoxelData");
		
		Menu->RemoveEntry(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser, MenuSection, "NWorldAssembly_ToggleCollisionVisualizer");
		Menu->RemoveEntry(NEXUS::CoreEditor::ToolMenus::LevelEditorToolBarUser, MenuSection, "NWorldAssembly_IgnoreSelectedActorsToggle");
	}
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

void FNWorldAssemblyEditorToolMenu::IgnoreSelectedActorsToggle()
{
	if (GetIgnoreSelectedActorsToggleMode() == 0)
	{
		// ADD
		USelection* AddSelection = GEditor->GetSelectedActors();
		for (FSelectionIterator It(*AddSelection); It; ++It)
		{
			AActor* Actor = Cast<AActor>(*It);
			Actor->Tags.Add(NEXUS::WorldAssembly::Tags::CellIgnoreActorTag);
		}
	}
	else
	{
		// REMOVE
		USelection* RemoveSelection = GEditor->GetSelectedActors();
		for (FSelectionIterator It(*RemoveSelection); It; ++It)
		{
			AActor* Actor = Cast<AActor>(*It);
			Actor->Tags.RemoveSwap(NEXUS::WorldAssembly::Tags::CellIgnoreActorTag);
		}
	}
}

bool FNWorldAssemblyEditorToolMenu::IgnoreSelectedActorsToggle_CanShow()
{
	return FNWorldAssemblyEdMode::HasCellActor() 
		&& FNEditorUtils::HasActorsSelected() 
		&& !FNWorldAssemblyEditorUtils::IsCellActorSelected();
}

int32 FNWorldAssemblyEditorToolMenu::GetIgnoreSelectedActorsToggleMode()
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
		
		if (Actor->ActorHasTag(NEXUS::WorldAssembly::Tags::CellIgnoreActorTag))
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
}

bool FNWorldAssemblyEditorToolMenu::Hull_SplitEdge_CanShow()
{
	
	// TODO: and selected edge?
	return FNWorldAssemblyEdMode::GetCellEdMode() == FNWorldAssemblyEdMode::ENCellEdMode::Hull;
}
