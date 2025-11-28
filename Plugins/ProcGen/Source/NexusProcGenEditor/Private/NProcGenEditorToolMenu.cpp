// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEditorToolMenu.h"

#include "NEditorCommands.h"
#include "NEditorUtilityWidget.h"
#include "Cell/NCellJunctionComponent.h"
#include "NProcGenRegistry.h"
#include "NEditorUtils.h"
#include "NProcGenDeveloperOverlayWidget.h"
#include "NProcGenEditorCommands.h"
#include "NProcGenEditorUtils.h"
#include "NProcGenEdMode.h"
#include "NProcGenSettings.h"
#include "WidgetBlueprint.h"

#define LOCTEXT_NAMESPACE "NexusProcGenEditor"

void FNProcGenEditorToolMenu::Register()
{
	// Level Tools
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User"))
	{
		FToolMenuSection& NexusSection = Menu->FindOrAddSection("NEXUS");
		
		// NOrgan Dropdown
		FToolMenuEntry NOrganDropdownMenu = FToolMenuEntry::InitComboButton(
			"NOrganExtensions_Button",
			FUIAction(
				FExecuteAction(),
				FCanExecuteAction::CreateStatic(&FNEditorUtils::IsNotPlayInEditor), // TODO: can we make this run in playmode?
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&FNProcGenEditorToolMenu::ShowOrganDropdown)),
				FOnGetContent::CreateLambda([]()
				{
					FMenuBuilder MenuBuilder(true, FNProcGenEditorCommands::Get().CommandList_Organ);
					MenuBuilder.SetSearchable(false); // Life's too short to search this menu.
					
					MenuBuilder.BeginSection("NOrganExtensions_SelectSection", LOCTEXT("NOrganExtensions_SelectSection", "Select Organ"));
					TArray<UNOrganComponent*> OrganComponents = FNProcGenRegistry::GetOrganComponentsFromLevel(FNEditorUtils::GetCurrentLevel());
					for (auto Organ : OrganComponents)
					{
						FText OrganName = FText::FromString(Organ->GetDebugLabel());
						
						MenuBuilder.AddMenuEntry(
							OrganName,
							FText::Format(LOCTEXT("NOrganExtensions_SelectNOrganComponent", "Select {0}"), OrganName),
							FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.SelectNCellJunctionComponent"),
							FUIAction(FExecuteAction::CreateStatic(&FNProcGenEditorCommands::OrganSelectComponent, Organ),
								FCanExecuteAction::CreateStatic(&FNProcGenEditorCommands::OrganSelectComponent_CanExecute, Organ),
								FIsActionChecked()));
						
					}
					MenuBuilder.EndSection();
					
					MenuBuilder.BeginSection("NOrganExtensions_ActionSection", LOCTEXT("NOrganExtensions_ActionSection", "Actions"));
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_OrganGenerate);
					MenuBuilder.EndSection();
					
					return MenuBuilder.MakeWidget();
				}),
			LOCTEXT("NOrganExtensions_Label", "Organ"),
			LOCTEXT("NOrganExtensions_ToolTip", "Making procedural content easier since 2017."),
			FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "ClassIcon.NOrganComponent")
		);
		NOrganDropdownMenu.StyleNameOverride = "CalloutToolbar";
		NexusSection.AddEntry(NOrganDropdownMenu);
		
		// Add a button that if a NCellActor/Pin is selected and were not in the ToolMode it will show and clicking switches mode
		const FToolMenuEntry NProcGenEdMode_Button = FToolMenuEntry::InitToolBarButton(
					"NProcGenEdMode_Button",
					FUIAction(
						FExecuteAction::CreateStatic(&FNProcGenEditorCommands::ProcGenEdMode),
						FCanExecuteAction::CreateStatic(&FNProcGenEditorCommands::ProcGenEdMode_CanExecute),
						FIsActionChecked(),
						FIsActionButtonVisible::CreateStatic(&FNProcGenEditorCommands::ProcGenEdMode_CanShow)),
						LOCTEXT("Command_NProcGenEdMode_Button", "Switch To NProcGen Editor Mode"),
						LOCTEXT("Command_NProcGenEdMode_Button", "Switch the current editor mode to the NProcGen Editor Mode, which enables specific tools for working with NCells, etc."),
						FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Icon.ProcGen"));
		NexusSection.AddEntry(NProcGenEdMode_Button);
		
		// Create our option Add NCellActor button that only shows in NProcGenEdMode + no present NCellActor
		const FToolMenuEntry NCellActor_AddButton = FToolMenuEntry::InitToolBarButton(
					"NCellActor_AddButton",
					FUIAction(
						FExecuteAction::CreateStatic(&FNProcGenEditorCommands::CellAddActor),
						FCanExecuteAction::CreateStatic(&FNProcGenEditorCommands::CellAddActor_CanExecute),
						FIsActionChecked(),
						FIsActionButtonVisible::CreateStatic(&FNProcGenEditorCommands::CellAddActor_CanShow)),
						LOCTEXT("Command_NCell_AddActor", "Add Actor"),
						LOCTEXT("Command_NCell_AddActor_Tooltip", "Create the singleton-like actor which will facilitate creating a NCell from the level it is placed in."),
						FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.AddNCellActor"));
		NexusSection.AddEntry(NCellActor_AddButton);

		// Create our option to select the NCellActor if we are in NProcGenEdMode and there is a NCellActor
		const FToolMenuEntry NCellActor_SelectButton = FToolMenuEntry::InitToolBarButton(
					"NCellActor_SelectButton",
					FUIAction(
						FExecuteAction::CreateStatic(&FNProcGenEditorCommands::CellSelectActor),
						FCanExecuteAction::CreateStatic(&FNProcGenEditorCommands::CellSelectActor_CanExecute),
						FIsActionChecked(),
						FIsActionButtonVisible::CreateStatic(&FNProcGenEditorCommands::CellSelectActor_CanShow)),
						LOCTEXT("Command_NCell_SelectActor", "Select Actor"),
						LOCTEXT("Command_NCell_SelectActor_Tooltip", "Select the NCellActor in the level."),
						FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.SelectNCellActor.Selected"));
		NexusSection.AddEntry(NCellActor_SelectButton);
		
		// Display EditMode toggle buttons
		FToolMenuEntry NCellActor_EditBoundsMode = FToolMenuEntry::InitToolBarButton(
					"NCellActor_EditBoundsMode",
					FUIAction(
						FExecuteAction::CreateStatic(&FNProcGenEditorCommands::CellActorEditBoundsMode),
						FCanExecuteAction(),
						FIsActionChecked(),
						FIsActionButtonVisible::CreateStatic(&FNProcGenEditorToolMenu::ShowCellEditMode)),
						LOCTEXT("Command_NCellActor_EditBoundsMode", "Edit Bounds"),
						LOCTEXT("Command_NCellActor_EditBoundsMode_Tooltip", "Edit the bounds of the NCell."),
						TAttribute<FSlateIcon>::Create(
							TAttribute<FSlateIcon>::FGetter::CreateStatic(
						&FNProcGenEditorCommands::CellActorEditBoundsMode_GetIcon)));
		NCellActor_EditBoundsMode.StyleNameOverride = "Toolbar.BackplateLeft";
		NexusSection.AddEntry(NCellActor_EditBoundsMode);
		
		FToolMenuEntry NCellActor_EditHullMode = FToolMenuEntry::InitToolBarButton(
			"NCellActor_EditHullMode",
			FUIAction(
				FExecuteAction::CreateStatic(&FNProcGenEditorCommands::CellActorEditHullMode),
				FCanExecuteAction::CreateStatic(&FNProcGenEditorCommands::CellActorEditHullMode_CanExecute),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&FNProcGenEditorToolMenu::ShowCellEditMode)),
				LOCTEXT("Command_NCellActor_EditHullMode", "Edit Hull"),
				LOCTEXT("Command_NCellActor_EditHullMode_Tooltip", "Edit the hull vertices of the NCell (requires a tri-based hull)."),
				TAttribute<FSlateIcon>::Create(
					TAttribute<FSlateIcon>::FGetter::CreateStatic(
				&FNProcGenEditorCommands::CellActorEditHullMode_GetIcon)));
		NCellActor_EditHullMode.StyleNameOverride = "Toolbar.BackplateCenter";
		NexusSection.AddEntry(NCellActor_EditHullMode);
		
		FToolMenuEntry NCellActor_EditVoxelMode = FToolMenuEntry::InitToolBarButton(
	"NCellActor_EditVoxelMode",
	FUIAction(
		FExecuteAction::CreateStatic(&FNProcGenEditorCommands::CellActorEditVoxelMode),
		FCanExecuteAction(),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateStatic(&FNProcGenEditorToolMenu::ShowCellEditMode)),
		LOCTEXT("Command_NCellActor_EditVoxelMode", "Edit Voxels"),
		LOCTEXT("Command_NCellActor_EditVoxelMode_Tooltip", "Edit the voxel points of the NCell."),
		TAttribute<FSlateIcon>::Create(
			TAttribute<FSlateIcon>::FGetter::CreateStatic(
		&FNProcGenEditorCommands::CellActorEditVoxelMode_GetIcon)));
		NCellActor_EditVoxelMode.StyleNameOverride = "Toolbar.BackplateRight";
		NexusSection.AddEntry(NCellActor_EditVoxelMode);
		
		// NCell Dropdown
		FToolMenuEntry NCellDropdownMenu = FToolMenuEntry::InitComboButton(
			"NCellExtensions_Button",
			FUIAction(
				FExecuteAction(),
				FCanExecuteAction::CreateStatic(&FNEditorUtils::IsNotPlayInEditor),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&FNProcGenEditorToolMenu::ShowCellDropdown)),
				FOnGetContent::CreateLambda([]()
				{
					FMenuBuilder MenuBuilder(true, FNProcGenEditorCommands::Get().CommandList_Cell);
					MenuBuilder.SetSearchable(false); // Life's too short to search this menu.
					
					MenuBuilder.BeginSection("NCellExtensions_Asset", LOCTEXT("NCellExtensions_AssetSection", "Asset"));
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_CellCaptureThumbnail);
					MenuBuilder.EndSection();
					
					MenuBuilder.BeginSection("NCellExtensions_CalculateSection", LOCTEXT("NCellExtensions_CalculateSection", "Calculate"));
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_CellCalculateAll);
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_CellCalculateBounds);
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_CellCalculateHull);
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_CellCalculateVoxelData);
					MenuBuilder.EndSection();
					
					MenuBuilder.BeginSection("NCellExtensions_QuickSettings", LOCTEXT("NCellExtensions_QuickSettings", "Quick Settings"));
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_CellToggleBoundsCalculateOnSave);
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_CellToggleHullCalculateOnSave);
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_CellToggleVoxelCalculateOnSave);
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_CellToggleVoxelData);
					MenuBuilder.EndSection();
					
					MenuBuilder.BeginSection("NCellExtensions_CleanupSection", LOCTEXT("NCellExtensions_CleanupSection", "Cleanup"));
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_CellResetCell);
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_CellRemoveActor);
					MenuBuilder.EndSection();
					
					return MenuBuilder.MakeWidget();
				}),
			LOCTEXT("NCellExtensions_Label", "Cell"),
			LOCTEXT("NCellExtensions_ToolTip", "Making procedural content easier since 2017."),
			FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "ClassIcon.NCellActor")
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
				FIsActionButtonVisible::CreateStatic(&FNProcGenEditorToolMenu::ShowCellJunctionDropdown)),
				FOnGetContent::CreateLambda([]()
				{
					FMenuBuilder MenuBuilder(true, FNProcGenEditorCommands::Get().CommandList_CellJunction);

				
					MenuBuilder.SetSearchable(false); // Life's too short to search this menu.
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_CellJunctionAddComponent);
					MenuBuilder.BeginSection("NCellJunctionExtensions_Section", LOCTEXT("NCellJunctionExtensions_Section", "Select Junction"));
					TArray<UNCellJunctionComponent*> Junctions = FNProcGenRegistry::GetCellJunctionsComponentsFromLevel(FNEditorUtils::GetCurrentLevel());
					for (auto Junction : Junctions)
					{
						FText JunctionName = FText::FromString(Junction->GetJunctionName());
						
						MenuBuilder.AddMenuEntry(
							JunctionName,
							FText::Format(LOCTEXT("SelectNCellJunctionComponent", "Select {0}"), JunctionName),
							FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.SelectNCellJunctionComponent"),
							FUIAction(FExecuteAction::CreateStatic(&FNProcGenEditorCommands::CellJunctionSelectComponent, Junction),
								FCanExecuteAction::CreateStatic(&FNProcGenEditorCommands::CellJunctionSelectComponent_CanExecute, Junction),
								FIsActionChecked()));
						
					}
					MenuBuilder.EndSection();
					
					return MenuBuilder.MakeWidget();
				}),
			LOCTEXT("NCellJunctionExtensions_Label", "Junction"),
			LOCTEXT("NCellJunctionExtensions_ToolTip", "Making procedural content easier since 2017."),
			FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "ClassIcon.NCellJunctionComponent")
		);
		NCellJunctionDropdownMenu.StyleNameOverride = "CalloutToolbar";
		NexusSection.AddEntry(NCellJunctionDropdownMenu);
		
		// Toggle Drawing Voxel Data
		FToolMenuEntry NCellActor_DrawVoxelData = FToolMenuEntry::InitToolBarButton(
			"NCellActor_DrawVoxelData",
			FUIAction(
				FExecuteAction::CreateStatic(&FNProcGenEditorCommands::CellActorToggleDrawVoxelData),
				FCanExecuteAction(),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&FNProcGenEditorToolMenu::ShowDrawVoxels)),
				LOCTEXT("Command_NCellActor_DrawVoxelData", "Draw Voxel Data"),
				LOCTEXT("Command_NCellActor_DrawVoxelData_Tooltip", "Toggle drawing calculated voxel data for the NCell."),
				TAttribute<FSlateIcon>::Create(
					TAttribute<FSlateIcon>::FGetter::CreateStatic(
				&FNProcGenEditorCommands::CellActorToggleDrawVoxelData_GetIcon)));
		NexusSection.AddEntry(NCellActor_DrawVoxelData);
	}

	// EUW Entry
	FToolMenuSection& MenuSection = FNEditorCommands::GetEditorUtilitiesMenuSection();
	FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry("NEXUS.ProcGen.System",  
		LOCTEXT("NProcGenSystemEditorUtilityWidget_OpenFull", "NProcGenSystem"),
		LOCTEXT("NProcGenSystemEditorUtilityWidget_Tip", "Opens the NProcGenSystem Window inside of an editor tab."),
		FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Icon.ProcGen"),
FUIAction(
FExecuteAction::CreateStatic(&FNProcGenEditorToolMenu::CreateEditorUtilityWindow),
		FCanExecuteAction::CreateStatic(&FNProcGenEditorToolMenu::CreateEditorUtilityWindow_CanExecute),
		FIsActionChecked(),
		FIsActionButtonVisible()));
	MenuSection.AddEntry(Entry);
}

bool FNProcGenEditorToolMenu::ShowCellEditMode()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;
	if (!FNProcGenEdMode::IsActive()) return false;
	
	return FNProcGenEditorUtils::IsCellActorSelected();
}

bool FNProcGenEditorToolMenu::ShowDrawVoxels()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;
	if (!FNProcGenEdMode::IsActive()) return false;
	if (!FNProcGenRegistry::HasRootComponents()) return false;
	return true;
}

bool FNProcGenEditorToolMenu::ShowCellDropdown()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	return FNProcGenEdMode::IsActive() && FNProcGenEdMode::HasCellActor();
}

bool FNProcGenEditorToolMenu::ShowCellJunctionDropdown()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	return FNProcGenEdMode::IsActive() && FNProcGenEdMode::HasCellActor();
}

bool FNProcGenEditorToolMenu::ShowOrganDropdown()
{
	return FNProcGenEdMode::IsActive() && FNProcGenRegistry::HasOrganComponents();
}

void FNProcGenEditorToolMenu::CreateEditorUtilityWindow()
{
	TSubclassOf<UNProcGenDeveloperOverlayWidget> WidgetClass = UNProcGenSettings::Get()->DeveloperOverlayWidget;
	if (WidgetClass == nullptr)
	{
		const FString TemplatePath = FString::Printf(TEXT("/Script/UMGEditor.WidgetBlueprint'/NexusProcGen/WB_NProcGenDeveloperOverlay.WB_NProcGenDeveloperOverlay'"));
		UWidgetBlueprint* TemplateWidget = LoadObject<UWidgetBlueprint>(nullptr, TemplatePath);
		
		WidgetClass = TemplateWidget->GeneratedClass;
	}
	UNEditorUtilityWidget::CreateFromWidget(WidgetClass, FName("NProcGenEditorUtilityWindow"));
}

bool FNProcGenEditorToolMenu::CreateEditorUtilityWindow_CanExecute()
{
	return !UNEditorUtilityWidget::HasEditorUtilityWidgetByName(FName("NProcGenEditorUtilityWindow"));
}

#undef LOCTEXT_NAMESPACE
