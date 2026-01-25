// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefsEditorToolMenu.h"

#include "NDynamicRefsEditorStyle.h"
#include "NEditorCommands.h"
#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetSystem.h"

void FNDynamicRefsEditorToolMenu::Register()
{
	// EUW Entry
	auto EditorWindow = FNWindowCommandInfo();
	
	EditorWindow.Identifier = "EUW_NDynamicRefs";
	EditorWindow.DisplayName = NSLOCTEXT("NexusProcGenEditor", "Create_EUW_DisplayName", "DynamicRefs");
	EditorWindow.Tooltip = NSLOCTEXT("NexusProcGenEditor", "Create_EUW_DisplayName", "Opens the NDynamicRefs Developer Overlay inside of an editor tab.");
	EditorWindow.Icon = FSlateIcon(FNDynamicRefsEditorStyle::GetStyleSetName(), "ClassIcon.NDynamicRefComponent");
	
	EditorWindow.Execute = FExecuteAction::CreateStatic(&FNDynamicRefsEditorToolMenu::CreateEditorUtilityWindow);
	EditorWindow.IsChecked = FIsActionChecked::CreateStatic(&FNDynamicRefsEditorToolMenu::HasEditorUtilityWindow);
	
	FNEditorCommands::AddWindowCommand(EditorWindow);
}

void FNDynamicRefsEditorToolMenu::CreateEditorUtilityWindow()
{
	UNEditorUtilityWidget::SpawnTab( TEXT("/NexusDynamicRefs/EditorResources/EUW_NDynamicRefs.EUW_NDynamicRefs"), FName("EUW_NDynamicRefs"));
}

bool FNDynamicRefsEditorToolMenu::HasEditorUtilityWindow()
{
	UNEditorUtilityWidgetSystem* System = UNEditorUtilityWidgetSystem::Get();
	if (System == nullptr) return false;
	return System->HasWidget(FName("EUW_NDynamicRefs"));
}