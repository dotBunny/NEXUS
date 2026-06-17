// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyEditorModule.h"
#include "AssetDefinitions/AssetDefinition_NCell.h"
#include "NWorldAssemblyEditorStyle.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "Macros/NModuleMacros.h"
#include "ComponentVisualizer.h"
// This is used by the save delegate to get a definition for FObjectPreSaveContext
// ReSharper disable once CppUnusedIncludeDirective
#include "UObject/ObjectSaveContext.h"
#include "EditorModeRegistry.h"
#include "IPlacementModeModule.h"
#include "NEditorDefaults.h"
#include "NPropertySections.h"
#include "Customizations/NCellRootComponentCustomization.h"
#include "Cell/NCellJunctionComponent.h"
#include "Visualizers/NCellJunctionComponentVisualizer.h"
#include "Cell/NCellProxy.h"
#include "Customizations/NCellProxyCustomization.h"
#include "Cell/NCellRootComponent.h"
#include "Organ/NOrganComponent.h"
#include "Visualizers/NCellRootComponentVisualizer.h"
#include "NWorldAssemblyEditorCommands.h"
#include "NWorldAssemblyEditorToolMenu.h"
#include "NWorldAssemblyEditorUndo.h"
#include "NWorldAssemblyEdMode.h"
#include "UnrealEdGlobals.h"
#include "Customizations/NOrganComponentCustomization.h"
#include "Editor/UnrealEdEngine.h"
#include "Macros/NEditorModuleMacros.h"
#include "Organ/NBoneActor.h"
#include "Organ/NBoneComponent.h"
#include "Organ/NOrganVolume.h"
#include "Visualizers/NBoneComponentVisualizer.h"

void FNWorldAssemblyEditorModule::StartupModule()
{
	N_MODULE_POST_ENGINE_INIT(FNWorldAssemblyEditorModule, OnPostEngineInit);
}

void FNWorldAssemblyEditorModule::ShutdownModule()
{
	N_MODULE_REMOVE_POST_ENGINE_INIT()
	// No-op in practice: the startup callback(s) are bound via CreateStatic (no `this` owner
	// for RemoveAll to match) and RegisterStartupCallback usually runs them immediately. Kept
	// for symmetry with Epic's module template; the real menu teardown is below.
	UToolMenus::UnRegisterStartupCallback(this);
	FEditorModeRegistry::Get().UnregisterMode(FNWorldAssemblyEdMode::Identifier);

	if (GUnrealEd)
	{
		RootComponentVisualizer.Reset();
		GUnrealEd->UnregisterComponentVisualizer(UNCellRootComponent::StaticClass()->GetFName());
		GUnrealEd->UnregisterComponentVisualizer(UNCellJunctionComponent::StaticClass()->GetFName());
		GUnrealEd->UnregisterComponentVisualizer(UNBoneComponent::StaticClass()->GetFName());
	}

	// Unregister customizations
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

		PropertyModule.UnregisterCustomClassLayout(UNCellJunctionComponent::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UNCellRootComponent::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(ANCellActor::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(ANCellProxy::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UNOrganComponent::StaticClass()->GetFName());

		PropertyModule.NotifyCustomizationModuleChanged();
	}
	
	N_UNREGISTER_PLACEABLE_ACTORS(PlacementActors)
	N_TOOLS_MENU_ENTRY_EUW_METHOD_UNREGISTER(EUW_NWorldAssemblySystem)();
	
	FNWorldAssemblyEditorToolMenu::RemoveMenuEntries();

	// Mirror the Register() in OnPostEngineInit. RemoveMenuEntries() above has already detached the global
	// actions that hold the command list alive, so it's now safe to drop the singleton. IsRegistered() skips
	// the headless cook/commandlet path, where Register() never ran.
	if (FNWorldAssemblyEditorCommands::IsRegistered())
	{
		FNWorldAssemblyEditorCommands::Unregister();
	}

	FNWorldAssemblyEditorStyle::Shutdown();

	// Remove Undo handler
	if (UndoHandler.IsValid())
	{
		UndoHandler.Reset();
	}
}

void FNWorldAssemblyEditorModule::OnPostEngineInit()
{
	// Initialize Undo Handler
	if (!UndoHandler.IsValid())
	{
		UndoHandler = MakeShared<FNWorldAssemblyEditorUndo>();
	}
	
	// Configure Style
	FNWorldAssemblyEditorStyle::Initialize();
	
	// Setup Asset Monitoring
	const FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	
	AssetRegistry.OnAssetRenamed().AddStatic(&UAssetDefinition_NCell::OnAssetRenamed);
	AssetRegistry.OnAssetRemoved().AddStatic(&UAssetDefinition_NCell::OnAssetRemoved);

	FEditorDelegates::PreSaveWorldWithContext.AddStatic(&UAssetDefinition_NCell::OnPreSaveWorldWithContext);

	// Initialize Tool Menu
	if (FSlateApplication::IsInitialized())
	{
		FNWorldAssemblyEditorCommands::Register();
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(&N_TOOLS_MENU_ENTRY_EUW_METHOD_REGISTER(EUW_NWorldAssemblySystem)));
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(FNWorldAssemblyEditorToolMenu::AddMenuEntries));
	}

	// Editor Mode
	FEditorModeRegistry::Get().RegisterMode<FNWorldAssemblyEdMode>(
		FNWorldAssemblyEdMode::Identifier,NSLOCTEXT("NexusWorldAssemblyEditor", "FNWorldAssemblyEdMode", "NEXUS: World Assembly"),
		FSlateIcon(FNWorldAssemblyEditorStyle::GetStyleSetName(), "Icon.WorldAssembly"), true);

	// Visualizers
	if (GUnrealEd != nullptr)
	{
		RootComponentVisualizer = MakeShared<FNCellRootComponentVisualizer>();
		GUnrealEd->RegisterComponentVisualizer(UNCellRootComponent::StaticClass()->GetFName(), RootComponentVisualizer);
		RootComponentVisualizer->OnRegister();
	
		const TSharedPtr<FComponentVisualizer> JunctionComponentVisualizer = MakeShared<FNCellJunctionComponentVisualizer>();
		GUnrealEd->RegisterComponentVisualizer(UNCellJunctionComponent::StaticClass()->GetFName(), JunctionComponentVisualizer);
		JunctionComponentVisualizer->OnRegister();
		
		const TSharedPtr<FComponentVisualizer> BoneComponentVisualizer = MakeShared<FNBoneComponentVisualizer>();
		GUnrealEd->RegisterComponentVisualizer(UNBoneComponent::StaticClass()->GetFName(), BoneComponentVisualizer);
		BoneComponentVisualizer->OnRegister();
	}

	// Register Customizations
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	PropertyModule.RegisterCustomClassLayout(ANCellProxy::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FNCellProxyCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(UNCellRootComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FNCellRootComponentCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(UNOrganComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FNOrganComponentCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
	
	// Handle Placement Definitions
	if (const FPlacementCategoryInfo* Info = FNEditorDefaults::GetPlacementCategory())
	{
		PlacementActors.Add(IPlacementModeModule::Get().RegisterPlaceableItem(Info->UniqueHandle, MakeShared<FPlaceableItem>(
		*ANBoneActor::StaticClass(),
		FAssetData(ANBoneActor::StaticClass()),
		NAME_None,
		NAME_None,
		TOptional<FLinearColor>(),
		TOptional<int32>(),
		NSLOCTEXT("NexusWorldAssemblyEditor", "NBoneActorPlacement", "Bone Actor"))));
		
		PlacementActors.Add(IPlacementModeModule::Get().RegisterPlaceableItem(Info->UniqueHandle, MakeShared<FPlaceableItem>(
		*ANOrganVolume::StaticClass(),
		FAssetData(ANOrganVolume::StaticClass()),
		NAME_None,
		NAME_None,
		TOptional<FLinearColor>(),
		TOptional<int32>(),
		NSLOCTEXT("NexusWorldAssemblyEditor", "NOrganVolumePlacement", "Organ Volume"))));
	}
	
	// Inspector Category Filter
	FNPropertySections::AddActorCategory("Cell Actor");
	FNPropertySections::AddActorCategory("Cell Proxy");
	FNPropertySections::AddActorCategory("Organ Volume");
	FNPropertySections::AddActorComponentCategory("Organ Component");
	FNPropertySections::AddSceneComponentCategory("Cell Root");
	FNPropertySections::AddSceneComponentCategory("Cell Junction");
	FNPropertySections::AddSceneComponentCategory("Bone Component");
}

IMPLEMENT_MODULE(FNWorldAssemblyEditorModule, NexusWorldAssemblyEditor)
