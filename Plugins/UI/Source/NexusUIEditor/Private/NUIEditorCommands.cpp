// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NUIEditorCommands.h"

#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetSubsystem.h"
#include "NUIEditorStyle.h"

void FNUIEditorCommands::AddMenuEntries()
{
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools.Debug"))
	{
		FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
			NEXUS::UIEditor::CollisionVisualizer::Identifier,
			NSLOCTEXT("NexusUIEditor", "Create_EUW_CollisionVisualizer_DisplayName", "Collision Visualizer"),
			NSLOCTEXT("NexusUIEditor", "Create_EUW_CollisionVisualizer_Tooltip", "Opens the NEXUS: Collision Visualizer window."),
			FSlateIcon(FNUIEditorStyle::GetStyleSetName(), NEXUS::UIEditor::CollisionVisualizer::Icon),
			FExecuteAction::CreateStatic(&FNUIEditorCommands::CreateCollisionVisualizerWindow));
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
			NEXUS::UIEditor::CollisionVisualizer::Identifier);
	}
}


void FNUIEditorCommands::CreateCollisionVisualizerWindow()
{
	UNEditorUtilityWidget::SpawnTab(
		NEXUS::UIEditor::CollisionVisualizer::Path, 
		NEXUS::UIEditor::CollisionVisualizer::Identifier);
}

bool FNUIEditorCommands::HasCollisionVisualizerWindow()
{
	UNEditorUtilityWidgetSubsystem* System = UNEditorUtilityWidgetSubsystem::Get();
	if (System == nullptr) return false;
	return System->HasWidget(NEXUS::UIEditor::CollisionVisualizer::Identifier);
}
