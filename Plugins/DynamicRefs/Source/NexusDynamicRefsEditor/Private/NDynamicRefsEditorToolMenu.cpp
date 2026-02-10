// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefsEditorToolMenu.h"

#include "NDynamicRefsEditorMinimal.h"
#include "NDynamicRefsEditorStyle.h"
#include "NEditorCommands.h"
#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetSystem.h"

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
	
	FNEditorCommands::AddWindowCommand(EditorWindow);
}

void FNDynamicRefsEditorToolMenu::Unregister()
{
	FNEditorCommands::RemoveWindowCommand(NEXUS::DynamicRefsEditor::EUW::Identifier);
}

void FNDynamicRefsEditorToolMenu::CreateEditorUtilityWindow()
{
	UNEditorUtilityWidget::SpawnTab(NEXUS::DynamicRefsEditor::EUW::Path, NEXUS::DynamicRefsEditor::EUW::Identifier);
}

bool FNDynamicRefsEditorToolMenu::HasEditorUtilityWindow()
{
	UNEditorUtilityWidgetSystem* System = UNEditorUtilityWidgetSystem::Get();
	if (System == nullptr) return false;
	return System->HasWidget(NEXUS::DynamicRefsEditor::EUW::Identifier);
}