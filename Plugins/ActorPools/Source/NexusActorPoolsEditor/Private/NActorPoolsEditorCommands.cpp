#include "NActorPoolsEditorCommands.h"

#include "BlueprintEditor.h"
#include "NActorPool.h"
#include "NActorPoolsEditorStyle.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Toolkits/AssetEditorToolkitMenuContext.h"

#define N_ACTOR_POOL_INVOKE_METHODS(Name, Method, Category) \
	UFunction* Name##Function = Blueprint->SkeletonGeneratedClass->FindFunctionByName(Method); \
	if (!Name##Function) \
	{ \
		UEdGraph* Name##FunctionGraph = FBlueprintEditorUtils::CreateNewGraph(Blueprint, \
			Method, UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass()); \
		FBlueprintEditorUtils::AddFunctionGraph<UFunction>(Blueprint, Name##FunctionGraph, true, nullptr); \
		FBlueprintEditorUtils::SetBlueprintFunctionOrMacroCategory(Name##FunctionGraph, Category); \
	}

void FNActorPoolsEditorCommands::AddMenuEntries()
{
	// Help Menu Submenu
	if (UToolMenu* AssetMenu = UToolMenus::Get()->ExtendMenu("AssetEditor.BlueprintEditor.MainMenu.Asset"))
	{
		FToolMenuSection& AssetSection = AssetMenu->FindOrAddSection(FName("AssetEditorActions"));
		AssetSection.AddDynamicEntry("NEXUS_AddActorPoolMethods", FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
		{
			UAssetEditorToolkitMenuContext* MenuContext = InSection.FindContext<UAssetEditorToolkitMenuContext>();
			if (MenuContext && MenuContext->Toolkit.IsValid())
			{
				TSharedPtr<FAssetEditorToolkit> Toolkit = MenuContext->Toolkit.Pin();
				if (Toolkit->IsActuallyAnAsset() && Toolkit->GetObjectsCurrentlyBeingEdited()->Num() == 1)
				{
					for (const UObject* EditedObject : *Toolkit->GetObjectsCurrentlyBeingEdited())
					{
						InSection.AddMenuEntry(
								FName("NEXUS_AddActorPoolMethods_Invoke"),
								NSLOCTEXT("NexusActorPools", "AddActorPoolMethods", "Add NActorPool Methods"),
								NSLOCTEXT("NexusActorPools", "AddActorPoolMethods_Tooltip", "Adds optional Actor Pool methods to the Blueprint."),
								FSlateIcon(FNActorPoolsEditorStyle::GetStyleSetName(), "ClassIcon.NActorPool"),
								FUIAction(
					FExecuteAction::CreateStatic(&FNActorPoolsEditorCommands::AddActorPoolMethods, EditedObject), 
					FCanExecuteAction())
							);
					}
				}
			}
		}));
	}
}

void FNActorPoolsEditorCommands::RemoveMenuEntries()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (ToolMenus)
	{
		ToolMenus->RemoveEntry("AssetEditor.BlueprintEditor.MainMenu.Asset", "AssetEditorActions", "NEXUS_AddActorPoolMethods");
	}
}

void FNActorPoolsEditorCommands::AddActorPoolMethods(const UObject* EditedObject)
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (AssetEditorSubsystem == nullptr) return;

	IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(const_cast<UObject*>(EditedObject), false);
	if (EditorInstance == nullptr) return;
	
	if (EditorInstance && EditorInstance->GetEditorName() == FName("BlueprintEditor"))
	{	
		FBlueprintEditor* BlueprintEditor = static_cast<FBlueprintEditor*>(EditorInstance);
		UBlueprint* Blueprint = BlueprintEditor->GetBlueprintObj();
		
		N_ACTOR_POOL_INVOKE_METHODS(OnCreated, NEXUS::ActorPools::InvokeMethods::OnCreated, NEXUS::ActorPools::InvokeMethods::Category)
		N_ACTOR_POOL_INVOKE_METHODS(OnSpawned, NEXUS::ActorPools::InvokeMethods::OnSpawned, NEXUS::ActorPools::InvokeMethods::Category)
		N_ACTOR_POOL_INVOKE_METHODS(OnReturn, NEXUS::ActorPools::InvokeMethods::OnReturn, NEXUS::ActorPools::InvokeMethods::Category)
		N_ACTOR_POOL_INVOKE_METHODS(OnDestroyed, NEXUS::ActorPools::InvokeMethods::OnDestroyed, NEXUS::ActorPools::InvokeMethods::Category)
		
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	}
}
