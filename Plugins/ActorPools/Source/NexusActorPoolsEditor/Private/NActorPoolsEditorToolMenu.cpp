#include "NActorPoolsEditorToolMenu.h"


#include "NActorPoolsEditorMinimal.h"
#include "NActorPoolsEditorStyle.h"
#include "NEditorToolsMenu.h"
#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetSubsystem.h"

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
	
	FNEditorToolsMenu::AddCommand(EditorWindow);
}

void FNActorPoolsEditorToolMenu::Unregister()
{
	FNEditorToolsMenu::RemoveCommand(NEXUS::ActorPoolsEditor::EditorUtilityWidget::Identifier);
}

void FNActorPoolsEditorToolMenu::CreateEditorUtilityWindow()
{
	UNEditorUtilityWidget::SpawnTab(
		NEXUS::ActorPoolsEditor::EditorUtilityWidget::Path, 
		NEXUS::ActorPoolsEditor::EditorUtilityWidget::Identifier);
}

bool FNActorPoolsEditorToolMenu::HasEditorUtilityWindow()
{
	UNEditorUtilityWidgetSubsystem* System = UNEditorUtilityWidgetSubsystem::Get();
	if (System == nullptr) return false;
	return System->HasWidget(NEXUS::ActorPoolsEditor::EditorUtilityWidget::Identifier);
}
