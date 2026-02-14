// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefsEditorToolMenu.h"

#include "NDynamicRefsEditorMinimal.h"
#include "NDynamicRefsEditorStyle.h"
#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetSubsystem.h"
#include "NToolsEditorMenu.h"

void FNDynamicRefsEditorToolMenu::Register()
{
	// EUW Entry
	auto EditorWindow = FNEditorCommandInfo();
	
	EditorWindow.Section = TEXT("Developer Overlay");
	EditorWindow.Identifier = NEXUS::DynamicRefsEditor::EUW::Identifier;
	EditorWindow.DisplayName = NSLOCTEXT("NexusProcGenEditor", "Create_EUW_DisplayName", "Dynamic References");
	EditorWindow.Tooltip = NSLOCTEXT("NexusProcGenEditor", "Create_EUW_Tooltip", "Opens the NDynamicRefs Developer Overlay inside of an editor tab.");
	EditorWindow.Icon = FSlateIcon(FNDynamicRefsEditorStyle::GetStyleSetName(), NEXUS::DynamicRefsEditor::EUW::Icon);
	EditorWindow.Execute = FExecuteAction::CreateStatic(&FNDynamicRefsEditorToolMenu::CreateEditorUtilityWindow);
	EditorWindow.IsChecked = FIsActionChecked::CreateStatic(&FNDynamicRefsEditorToolMenu::HasEditorUtilityWindow);
	
	FNToolsEditorMenu::AddCommand(EditorWindow);
}

void FNDynamicRefsEditorToolMenu::Unregister()
{
	FNToolsEditorMenu::RemoveCommand(NEXUS::DynamicRefsEditor::EUW::Identifier);
}

void FNDynamicRefsEditorToolMenu::CreateEditorUtilityWindow()
{
	UNEditorUtilityWidget::SpawnTab(NEXUS::DynamicRefsEditor::EUW::Path, NEXUS::DynamicRefsEditor::EUW::Identifier);
}

bool FNDynamicRefsEditorToolMenu::HasEditorUtilityWindow()
{
	UNEditorUtilityWidgetSubsystem* System = UNEditorUtilityWidgetSubsystem::Get();
	if (System == nullptr) return false;
	return System->HasWidget(NEXUS::DynamicRefsEditor::EUW::Identifier);
}