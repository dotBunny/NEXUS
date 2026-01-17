#include "NActorPoolsEditorToolMenu.h"

#include "NActorPoolsDeveloperOverlayWidget.h"
#include "NActorPoolsEditorStyle.h"
#include "NActorPoolsSettings.h"
#include "NEditorCommands.h"
#include "NWidgetEditorUtilityWidget.h"

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
	// Default value
	const TSubclassOf<UNActorPoolsDeveloperOverlayWidget> WidgetClass = UNActorPoolsSettings::Get()->DeveloperOverlayWidget;
	
	// Evaluate override
	if (WidgetClass != nullptr)
	{
		FString PathName = WidgetClass->GetPathName();
		PathName.RemoveFromEnd("_C");
		
		UNWidgetEditorUtilityWidget::GetOrCreate(
		EditorUtilityWindowName, 
		 FString::Printf(TEXT("/Script/UMGEditor.WidgetBlueprint'%s'"), *PathName), 
		FText::FromString("Actor Pools"), 
		FNActorPoolsEditorStyle::GetStyleSetName(), "ClassIcon.NActorPool");
	}
}

bool FNActorPoolsEditorToolMenu::HasEditorUtilityWindow()
{
	return UNWidgetEditorUtilityWidget::HasEditorUtilityWidget(EditorUtilityWindowName);
}
