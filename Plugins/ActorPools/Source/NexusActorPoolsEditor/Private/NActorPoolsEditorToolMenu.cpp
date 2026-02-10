#include "NActorPoolsEditorToolMenu.h"


#include "NActorPoolsEditorMinimal.h"
#include "NActorPoolsEditorStyle.h"
#include "NEditorCommands.h"
#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetSystem.h"

void FNActorPoolsEditorToolMenu::Register()
{
	// EUW Entry
	auto EditorWindow = FNEditorCommandInfo();
	
	EditorWindow.Section = TEXT("Developer Overlay");
	EditorWindow.Identifier = NEXUS::ActorPoolsEditor::EditorUtilityWidget::Identifier;
	EditorWindow.DisplayName = NSLOCTEXT("NexusActorPoolsEditor", "Create_EUW_DisplayName", "Actor Pools");
	EditorWindow.Tooltip = NSLOCTEXT("NexusActorPoolsEditor", "Create_EUW_Tooltip", "Opens the NActorPools Developer Overlay inside of an editor tab.");
	EditorWindow.Icon = FSlateIcon(FNActorPoolsEditorStyle::GetStyleSetName(), NEXUS::ActorPoolsEditor::EditorUtilityWidget::Icon);
	
	EditorWindow.Execute = FExecuteAction::CreateStatic(&FNActorPoolsEditorToolMenu::CreateEditorUtilityWindow);
	EditorWindow.IsChecked = FIsActionChecked::CreateStatic(&FNActorPoolsEditorToolMenu::HasEditorUtilityWindow);
	
	FNEditorCommands::AddWindowCommand(EditorWindow);
}

void FNActorPoolsEditorToolMenu::Unregister()
{
	FNEditorCommands::RemoveWindowCommand(NEXUS::ActorPoolsEditor::EditorUtilityWidget::Identifier);
}

void FNActorPoolsEditorToolMenu::CreateEditorUtilityWindow()
{
	UNEditorUtilityWidget::SpawnTab(
		NEXUS::ActorPoolsEditor::EditorUtilityWidget::Path, 
		NEXUS::ActorPoolsEditor::EditorUtilityWidget::Identifier);
}

bool FNActorPoolsEditorToolMenu::HasEditorUtilityWindow()
{
	UNEditorUtilityWidgetSystem* System = UNEditorUtilityWidgetSystem::Get();
	if (System == nullptr) return false;
	return System->HasWidget(NEXUS::ActorPoolsEditor::EditorUtilityWidget::Identifier);
}
