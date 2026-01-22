#include "NActorPoolsEditorToolMenu.h"


#include "NActorPoolsEditorStyle.h"
#include "NEditorCommands.h"
#include "NEditorUtilityWidget.h"
#include "NEditorUtilityWidgetSystem.h"


FName FNActorPoolsEditorToolMenu::EditorUtilityWindowName = FName("NActorPoolsEditorUtilityWindow");

void FNActorPoolsEditorToolMenu::Register()
{
	// EUW Entry
	auto EditorWindow = FNWindowCommandInfo();
	
	EditorWindow.Identifier = "NActorPools";
	EditorWindow.DisplayName = NSLOCTEXT("NexusActorPoolsEditor", "Create_EUW_DisplayName", "Actor Pools");
	EditorWindow.Tooltip = NSLOCTEXT("NexusActorPoolsEditor", "Create_EUW_DisplayName", "Opens the NActorPools Developer Overlay inside of an editor tab.");
	EditorWindow.Icon = FSlateIcon(FNActorPoolsEditorStyle::GetStyleSetName(), "ClassIcon.NActorPool");
	
	EditorWindow.Execute = FExecuteAction::CreateStatic(&FNActorPoolsEditorToolMenu::CreateEditorUtilityWindow);
	EditorWindow.IsChecked = FIsActionChecked::CreateStatic(&FNActorPoolsEditorToolMenu::HasEditorUtilityWindow);
	
	FNEditorCommands::AddWindowCommand(EditorWindow);
}

void FNActorPoolsEditorToolMenu::CreateEditorUtilityWindow()
{
	UNEditorUtilityWidget::SpawnTab(TEXT("/NexusActorPools/EditorResources/EUW_NActorPools.EUW_NActorPools"), FName("EUW_NActorPools"));
}

bool FNActorPoolsEditorToolMenu::HasEditorUtilityWindow()
{
	UNEditorUtilityWidgetSystem* System = UNEditorUtilityWidgetSystem::Get();
	if (System == nullptr) return false;
	return System->HasWidget(FName("EUW_NActorPools"));
}
