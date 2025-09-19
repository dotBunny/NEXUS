// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEditorToolMenu.h"

#include "Cell/NCellJunctionComponent.h"
#include "Cell/NCellRegistry.h"
#include "NEditorUtils.h"
#include "NProcGenEditorCommands.h"
#include "NProcGenEditorUtils.h"
#include "NProcGenEdMode.h"

#define LOCTEXT_NAMESPACE "NexusProcGenEditor"

void FNProcGenEditorToolMenu::Register()
{
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User"))
	{
		FToolMenuSection& NexusSection = Menu->FindOrAddSection("NEXUS");

		// Add a button that if a NCellActor/Pin is selected and were not in the ToolMode it will show and clicking switches mode
		const FToolMenuEntry NProcGenEdMode_Button = FToolMenuEntry::InitToolBarButton(
					"NProcGenEdMode_Button",
					FUIAction(
						FExecuteAction::CreateStatic(&FNProcGenEditorCommands::NProcGenEdMode),
						FCanExecuteAction::CreateStatic(&FNProcGenEditorCommands::NProcGenEdMode_CanExecute),
						FIsActionChecked(),
						FIsActionButtonVisible::CreateStatic(&FNProcGenEditorCommands::NProcGenEdMode_CanShow)),
						LOCTEXT("Command_NProcGenEdMode_Button", "Switch To NProcGen Editor Mode"),
						LOCTEXT("Command_NProcGenEdMode_Button", "Switch the current editor mode to the NProcGen Editor Mode, which enables specific tools for working with NCells, etc."),
						FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Icon.ProcGen"));
		NexusSection.AddEntry(NProcGenEdMode_Button);
		
		// Create our option Add NCellActor button that only shows in NProcGenEdMode + no present NCellActor
		const FToolMenuEntry NCellActor_AddButton = FToolMenuEntry::InitToolBarButton(
					"NCellActor_AddButton",
					FUIAction(
						FExecuteAction::CreateStatic(&FNProcGenEditorCommands::NCellAddActor),
						FCanExecuteAction::CreateStatic(&FNProcGenEditorCommands::NCellAddActor_CanExecute),
						FIsActionChecked(),
						FIsActionButtonVisible::CreateStatic(&FNProcGenEditorCommands::NCellAddActor_CanShow)),
						LOCTEXT("Command_NCell_AddActor", "Add Actor"),
						LOCTEXT("Command_NCell_AddActor_Tooltip", "Create the singleton-like actor which will facilitate creating a NCell from the level it is placed in."),
						FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.AddNCellActor"));
		NexusSection.AddEntry(NCellActor_AddButton);

		// Create our option to select the NCellActor if we are in NProcGenEdMode and there is a NCellActor
		const FToolMenuEntry NCellActor_SelectButton = FToolMenuEntry::InitToolBarButton(
					"NCellActor_SelectButton",
					FUIAction(
						FExecuteAction::CreateStatic(&FNProcGenEditorCommands::NCellSelectActor),
						FCanExecuteAction::CreateStatic(&FNProcGenEditorCommands::NCellSelectActor_CanExecute),
						FIsActionChecked(),
						FIsActionButtonVisible::CreateStatic(&FNProcGenEditorCommands::NCellSelectActor_CanShow)),
						LOCTEXT("Command_NCell_SelectActor", "Select Actor"),
						LOCTEXT("Command_NCell_SelectActor_Tooltip", "Select the NCellActor in the level."),
						FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.SelectNCellActor"));
		NexusSection.AddEntry(NCellActor_SelectButton);
		
		// NCell Dropdown
		FToolMenuEntry NCellDropdownMenu = FToolMenuEntry::InitComboButton(
			"NCellExtensions_Button",
			FUIAction(
				FExecuteAction(),
				FCanExecuteAction::CreateStatic(&FNEditorUtils::IsNotPlayInEditor),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&FNProcGenEditorToolMenu::ShowNCellDropdown)),
				FOnGetContent::CreateLambda([]()
				{
					FMenuBuilder MenuBuilder(true, FNProcGenEditorCommands::Get().CommandList_NCell);
					MenuBuilder.SetSearchable(false); // Life's to short to search this menu.
					MenuBuilder.BeginSection("NCellExtensions_CalculateSection", LOCTEXT("NCellExtensions_CalculateSection", "Calculate"));
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_NCellCalculateAll);
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_NCellCalculateBounds);
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_NCellCalculateHull);
					MenuBuilder.EndSection();
					MenuBuilder.BeginSection("NCellExtensions_CleanupSection", LOCTEXT("NCellExtensions_CleanupSection", "Cleanup"));
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_NCellResetCell);
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_NCellRemoveActor);
					MenuBuilder.EndSection();
					return MenuBuilder.MakeWidget();
				}),
			LOCTEXT("NCellExtensions_Label", "Cell"),
			LOCTEXT("NCellExtensions_ToolTip", "Making procedural content easier since 2017."),
			FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "ClassIcon.NCellActor")
		);
		NCellDropdownMenu.StyleNameOverride = "CalloutToolbar";
		NexusSection.AddEntry(NCellDropdownMenu);

		// Display EditMode toggle buttons
		FToolMenuEntry NCellActor_EditBoundsMode = FToolMenuEntry::InitToolBarButton(
					"NCellActor_EditBoundsMode",
					FUIAction(
						FExecuteAction::CreateStatic(&FNProcGenEditorCommands::NCellActorEditBoundsMode),
						FCanExecuteAction(),
						FIsActionChecked(),
						FIsActionButtonVisible::CreateStatic(&FNProcGenEditorToolMenu::ShowNCellEditMode)),
						LOCTEXT("Command_NCellActor_EditBoundsMode", "Edit Bounds"),
						LOCTEXT("Command_NCellActor_EditBoundsMode_Tooltip", "Edit the bounds of the NCell."),
						TAttribute<FSlateIcon>::Create(
							TAttribute<FSlateIcon>::FGetter::CreateStatic(
						&FNProcGenEditorCommands::NCellActorEditBoundsMode_GetIcon)));
		NCellActor_EditBoundsMode.StyleNameOverride = "Toolbar.BackplateLeft";
		NexusSection.AddEntry(NCellActor_EditBoundsMode);
		FToolMenuEntry NCellActor_EditHullMode = FToolMenuEntry::InitToolBarButton(
			"NCellActor_EditHullMode",
			FUIAction(
				FExecuteAction::CreateStatic(&FNProcGenEditorCommands::NCellActorEditHullMode),
				FCanExecuteAction(),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&FNProcGenEditorToolMenu::ShowNCellEditMode)),
				LOCTEXT("Command_NCellActor_EditHullMode", "Edit Hull"),
				LOCTEXT("Command_NCellActor_EditHullMode_Tooltip", "Edit the hull vertices of the NCell."),
				TAttribute<FSlateIcon>::Create(
					TAttribute<FSlateIcon>::FGetter::CreateStatic(
				&FNProcGenEditorCommands::NCellActorEditHullMode_GetIcon)));
		NCellActor_EditHullMode.StyleNameOverride = "Toolbar.BackplateRight";
		NexusSection.AddEntry(NCellActor_EditHullMode);
		
		// NCellJunction Dropdown
		FToolMenuEntry NCellJunctionDropdownMenu = FToolMenuEntry::InitComboButton(
			"NCellJunctionExtensions_Button",
			FUIAction(
				FExecuteAction(),
				FCanExecuteAction::CreateStatic(&FNEditorUtils::IsNotPlayInEditor),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&FNProcGenEditorToolMenu::ShowNCellJunctionDropdown)),
				FOnGetContent::CreateLambda([]()
				{
					FMenuBuilder MenuBuilder(true, FNProcGenEditorCommands::Get().CommandList_NCellJunction);

				
					MenuBuilder.SetSearchable(false); // Life's to short to search this menu.
					MenuBuilder.AddMenuEntry(FNProcGenEditorCommands::Get().CommandInfo_NCellJunctionAddComponent);
					MenuBuilder.BeginSection("NCellJunctionExtensions_Section", LOCTEXT("NCellJunctionExtensions_Section", "Select Junction"));
					TArray<UNCellJunctionComponent*> Junctions = FNCellRegistry::GetJunctionsComponentsFromLevel(FNEditorUtils::GetCurrentLevel());
					for (auto Junction : Junctions)
					{
						FText JunctionName = FText::FromString(Junction->GetJunctionName());
						
						MenuBuilder.AddMenuEntry(
							JunctionName,
							FText::Format(NSLOCTEXT("NexusProcGenEditor", "SelectNCellJunctionComponent", "Select {0}"), JunctionName),
							FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Command.ProGenEd.SelectNCellJunctionComponent"),
							FUIAction(FExecuteAction::CreateStatic(&FNProcGenEditorCommands::OnNCellJunctionSelectComponent, Junction),
								FCanExecuteAction::CreateStatic(&FNProcGenEditorCommands::OnNCellJunctionSelectComponent_CanExecute, Junction),
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
	}
}

bool FNProcGenEditorToolMenu::ShowNCellEditMode()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;
	if (!FNProcGenEdMode::IsActive()) return false;
	
	return FNProcGenEditorUtils::IsNCellActorSelected();
}

bool FNProcGenEditorToolMenu::ShowNCellDropdown()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	return FNProcGenEdMode::IsActive() && FNProcGenEdMode::HasNCellActor();
}

bool FNProcGenEditorToolMenu::ShowNCellJunctionDropdown()
{
	if (FNEditorUtils::IsPlayInEditor()) return false;

	return FNProcGenEdMode::IsActive() && FNProcGenEdMode::HasNCellActor();
}

#undef LOCTEXT_NAMESPACE
