// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NUIEditorToolMenu.h"

#include "NEditorCommands.h"
#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetSystem.h"
#include "NUIEditorStyle.h"

void FNUIEditorToolMenu::Register()
{
	// EUW Entry
	auto EditorWindow = FNEditorCommandInfo();
	
	EditorWindow.Section = TEXT("Windows");
	EditorWindow.Identifier = NEXUS::UIEditor::CollisionQueryTest::Identifier;
	EditorWindow.DisplayName = NSLOCTEXT("NexusUIEditor", "Create_EUW_CollisionQueryTest_DisplayName", "Collision Query");
	EditorWindow.Tooltip = NSLOCTEXT("NexusUIEditor", "Create_EUW_CollisionQueryTest_Tooltip", "Opens the Collision Query Test window.");
	EditorWindow.Icon = FSlateIcon(FNUIEditorStyle::GetStyleSetName(), NEXUS::UIEditor::CollisionQueryTest::Icon);
	
	EditorWindow.Execute = FExecuteAction::CreateStatic(&FNUIEditorToolMenu::CreateCollisionQueryWindow);
	EditorWindow.IsChecked = FIsActionChecked::CreateStatic(&FNUIEditorToolMenu::HasCollisionQueryWindow);
	
	FNEditorCommands::AddToolCommand(EditorWindow);
}

void FNUIEditorToolMenu::Unregister()
{
	FNEditorCommands::RemoveWindowCommand(NEXUS::UIEditor::CollisionQueryTest::Identifier);
}

void FNUIEditorToolMenu::CreateCollisionQueryWindow()
{
	UNEditorUtilityWidget::SpawnTab(
		NEXUS::UIEditor::CollisionQueryTest::Path, 
		NEXUS::UIEditor::CollisionQueryTest::Identifier);
}

bool FNUIEditorToolMenu::HasCollisionQueryWindow()
{
	UNEditorUtilityWidgetSystem* System = UNEditorUtilityWidgetSystem::Get();
	if (System == nullptr) return false;
	return System->HasWidget(NEXUS::UIEditor::CollisionQueryTest::Identifier);
}
