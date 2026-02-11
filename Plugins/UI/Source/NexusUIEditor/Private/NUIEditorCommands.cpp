// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NUIEditorCommands.h"

#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetSystem.h"
#include "NUIEditorStyle.h"

void FNUIEditorCommands::AddMenuEntries()
{
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools.Debug"))
	{
		FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
			NEXUS::UIEditor::CollisionQueryTest::Identifier,
			NSLOCTEXT("NexusUIEditor", "Create_EUW_CollisionQueryTest_DisplayName", "Collision Query"),
			NSLOCTEXT("NexusUIEditor", "Create_EUW_CollisionQueryTest_Tooltip", "Opens the NEXUS: Collision Query Test window."),
			FSlateIcon(FNUIEditorStyle::GetStyleSetName(), NEXUS::UIEditor::CollisionQueryTest::Icon),
			FExecuteAction::CreateStatic(&FNUIEditorCommands::CreateCollisionQueryWindow));
		Entry.InsertPosition = FToolMenuInsert(NAME_None, EToolMenuInsertType::First);
		Menu->AddMenuEntry("Debug", Entry);
	}
}

void FNUIEditorCommands::RemoveMenuEntries()
{
	UToolMenus* Menu = UToolMenus::Get();
	if (Menu)
	{
		Menu->RemoveEntry("LevelEditor.MainMenu.Tools.Debug", "Debug", 
			NEXUS::UIEditor::CollisionQueryTest::Identifier);
	}
}


void FNUIEditorCommands::CreateCollisionQueryWindow()
{
	UNEditorUtilityWidget::SpawnTab(
		NEXUS::UIEditor::CollisionQueryTest::Path, 
		NEXUS::UIEditor::CollisionQueryTest::Identifier);
}

bool FNUIEditorCommands::HasCollisionQueryWindow()
{
	UNEditorUtilityWidgetSystem* System = UNEditorUtilityWidgetSystem::Get();
	if (System == nullptr) return false;
	return System->HasWidget(NEXUS::UIEditor::CollisionQueryTest::Identifier);
}
