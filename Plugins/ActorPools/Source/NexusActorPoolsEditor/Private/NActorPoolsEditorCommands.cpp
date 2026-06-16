// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolsEditorCommands.h"

#include "BlueprintEditor.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "INActorPoolItem.h"
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
					const UObject* EditedObject = (*Toolkit->GetObjectsCurrentlyBeingEdited())[0];

					// Only offer the actor pool methods on Actor-derived Blueprints. The runtime InvokeUFunctions
					// path is invoked exclusively on pooled Actors, so adding these stubs to a non-Actor Blueprint
					// (e.g. a plain UObject or data asset Blueprint) produces inert graphs that are never called.
					const UBlueprint* Blueprint = Cast<UBlueprint>(EditedObject);
					if (Blueprint == nullptr || Blueprint->ParentClass == nullptr ||
						!Blueprint->ParentClass->IsChildOf(AActor::StaticClass()))
					{
						return;
					}

					// Actors implementing INActorPoolItem take the fast Cast<INActorPoolItem>() dispatch path and
					// never reach InvokeUFunctions, so the stub methods would be dead weight. The interface is
					// CannotImplementInterfaceInBlueprint (native C++ only), so checking the parent class is sufficient.
					if (Blueprint->ParentClass->ImplementsInterface(UNActorPoolItem::StaticClass()))
					{
						return;
					}

					InSection.AddMenuEntry(
								FName("NEXUS_AddActorPoolMethods_Invoke"),
								NSLOCTEXT("NexusActorPools", "AddActorPoolMethods", "Add NActorPool Methods"),
								NSLOCTEXT("NexusActorPools", "AddActorPoolMethods_Tooltip", "Adds optional Actor Pool methods to the Blueprint."),
								FSlateIcon(FNActorPoolsEditorStyle::GetStyleSetName(), "ClassIcon.NActorPool"),
								FUIAction(FExecuteAction::CreateStatic(&FNActorPoolsEditorCommands::AddActorPoolMethods, EditedObject), 
								FCanExecuteAction())
							);
				}
			}
		}));
	}
}

void FNActorPoolsEditorCommands::RemoveMenuEntries()
{
	UToolMenus* ToolMenus = UToolMenus::TryGet();
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

		// Guard against non-Actor Blueprints reaching this public entry point; the methods are meaningless
		// outside the pooled-Actor InvokeUFunctions path. Actors that natively implement INActorPoolItem take
		// the fast dispatch path instead, so the stubs would be dead weight for them too.
		if (Blueprint == nullptr || Blueprint->ParentClass == nullptr ||
			Blueprint->SkeletonGeneratedClass == nullptr ||
			!Blueprint->ParentClass->IsChildOf(AActor::StaticClass()) ||
			Blueprint->ParentClass->ImplementsInterface(UNActorPoolItem::StaticClass()))
		{
			return;
		}

		N_ACTOR_POOL_INVOKE_METHODS(OnCreated, NEXUS::ActorPools::InvokeMethods::OnCreatedByActorPool, NEXUS::ActorPools::InvokeMethods::Category)
		N_ACTOR_POOL_INVOKE_METHODS(OnSpawned, NEXUS::ActorPools::InvokeMethods::OnSpawnedFromActorPool, NEXUS::ActorPools::InvokeMethods::Category)
		N_ACTOR_POOL_INVOKE_METHODS(OnReturn, NEXUS::ActorPools::InvokeMethods::OnReturnToActorPool, NEXUS::ActorPools::InvokeMethods::Category)
		N_ACTOR_POOL_INVOKE_METHODS(OnReleased, NEXUS::ActorPools::InvokeMethods::OnReleasedFromActorPool, NEXUS::ActorPools::InvokeMethods::Category)
		
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	}
}
