// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NGuardianEditorToolMenu.h"

#include "NGuardianEditorMinimal.h"
#include "NGuardianEditorStyle.h"
#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetSubsystem.h"
#include "Menus/NToolsMenu.h"

void FNGuardianEditorToolMenu::Register()
{
	// EUW Entry
	auto EditorWindow = FNMenuEntry();
	
	EditorWindow.Section = TEXT("Developer Overlay");
	EditorWindow.Identifier = NEXUS::Guardian::EUW::Identifier;
	EditorWindow.DisplayName = NSLOCTEXT("NexusGuardianEditor", "Create_EUW_DisplayName", "Guardian");
	EditorWindow.Tooltip = NSLOCTEXT("NexusGuardianEditor", "Create_EUW_Tooltip", "Opens the NGuardian Developer Overlay inside of an editor tab.");
	EditorWindow.Icon = FSlateIcon(FNGuardianEditorStyle::GetStyleSetName(), NEXUS::Guardian::EUW::Icon);
	EditorWindow.Execute = FExecuteAction::CreateStatic(&FNGuardianEditorToolMenu::CreateEditorUtilityWindow);
	EditorWindow.IsChecked = FIsActionChecked::CreateStatic(&FNGuardianEditorToolMenu::HasEditorUtilityWindow);
	
	FNToolsMenu::AddMenuEntry(EditorWindow);
}

void FNGuardianEditorToolMenu::Unregister()
{
	FNToolsMenu::RemoveMenuEntry(NEXUS::Guardian::EUW::Identifier);
}

void FNGuardianEditorToolMenu::CreateEditorUtilityWindow()
{
	UNEditorUtilityWidget::SpawnTab(NEXUS::Guardian::EUW::Path, NEXUS::Guardian::EUW::Identifier);
}

bool FNGuardianEditorToolMenu::HasEditorUtilityWindow()
{
	UNEditorUtilityWidgetSubsystem* System = UNEditorUtilityWidgetSubsystem::Get();
	if (System == nullptr) return false;
	return System->HasWidget(NEXUS::Guardian::EUW::Identifier);
}