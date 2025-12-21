// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenEditorModule.h"
#include "AssetDefinitions/AssetDefinition_NCell.h"
#include "NProcGenEditorStyle.h"
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
#include "Customizations/NCellRootComponentCustomization.h"
#include "Cell/NCellJunctionComponent.h"
#include "Visualizers/NCellJunctionComponentVisualizer.h"
#include "Cell/NCellProxy.h"
#include "Customizations/NCellProxyCustomization.h"
#include "Cell/NCellRootComponent.h"
#include "Organ/NOrganComponent.h"
#include "Visualizers/NCellRootComponentVisualizer.h"
#include "NProcGenEditorCommands.h"
#include "NProcGenEditorMinimal.h"
#include "NProcGenEditorToolMenu.h"
#include "NProcGenEditorUndo.h"
#include "NProcGenEdMode.h"
#include "UnrealEdGlobals.h"
#include "Customizations/NOrganComponentCustomization.h"
#include "Editor/UnrealEdEngine.h"
#include "Macros/NEditorModuleMacros.h"
#include "Organ/NBoneActor.h"
#include "Organ/NBoneComponent.h"
#include "Organ/NOrganVolume.h"
#include "Visualizers/NBoneComponentVisualizer.h"

void FNProcGenEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNProcGenEditorModule, OnPostEngineInit);
}

void FNProcGenEditorModule::ShutdownModule()
{
	FEditorModeRegistry::Get().UnregisterMode(FNProcGenEdMode::Identifier);

	if (GUnrealEd)
	{
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
	
	N_IMPLEMENT_UNREGISTER_PLACEABLE_ACTORS(PlacementActors)
	
	FNProcGenEditorStyle::Shutdown();

	// Remove Undo handler
	if (UndoHandler != nullptr)
	{
		delete UndoHandler;
		UndoHandler = nullptr;
	}
}

void FNProcGenEditorModule::OnPostEngineInit()
{
	// Initialize Undo Handler
	if (UndoHandler == nullptr)
	{
		UndoHandler = new FNProcGenEditorUndo();
	}
	
	// Configure Style
	FNProcGenEditorStyle::Initialize();
	
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
		FNProcGenEditorCommands::Register();
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(FNProcGenEditorToolMenu::Register));
	}

	// Editor Mode
	FEditorModeRegistry::Get().RegisterMode<FNProcGenEdMode>(
		FNProcGenEdMode::Identifier,NSLOCTEXT("NexusProcGenEditor", "FNProcGenEdMode", "NEXUS: ProcGen"),
		FSlateIcon(FNProcGenEditorStyle::GetStyleSetName(), "Icon.ProcGen"), true);

	// Visualizers
	if (GUnrealEd != nullptr)
	{
		const TSharedPtr<FComponentVisualizer> RootComponentVisualizer = MakeShareable(new FNCellRootComponentVisualizer());
		GUnrealEd->RegisterComponentVisualizer(UNCellRootComponent::StaticClass()->GetFName(), RootComponentVisualizer);
		RootComponentVisualizer->OnRegister();
	
		const TSharedPtr<FComponentVisualizer> JunctionComponentVisualizer = MakeShareable(new FNCellJunctionComponentVisualizer());
		GUnrealEd->RegisterComponentVisualizer(UNCellJunctionComponent::StaticClass()->GetFName(), JunctionComponentVisualizer);
		JunctionComponentVisualizer->OnRegister();
		
		const TSharedPtr<FComponentVisualizer> BoneComponentVisualizer = MakeShareable(new FNBoneComponentVisualizer());
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
		NSLOCTEXT("NexusProcGenEditor", "NBoneActorPlacement", "Bone Actor"))));
		
		PlacementActors.Add(IPlacementModeModule::Get().RegisterPlaceableItem(Info->UniqueHandle, MakeShared<FPlaceableItem>(
		*ANOrganVolume::StaticClass(),
		FAssetData(ANOrganVolume::StaticClass()),
		NAME_None,
		NAME_None,
		TOptional<FLinearColor>(),
		TOptional<int32>(),
		NSLOCTEXT("NexusProcGenEditor", "NOrganVolumePlacement", "Organ Volume"))));
		
		PlacementActors.Add(IPlacementModeModule::Get().RegisterPlaceableItem(Info->UniqueHandle, MakeShared<FPlaceableItem>(
			*ANCellJunctionBlockerActor::StaticClass(),
			FAssetData(ANCellJunctionBlockerActor::StaticClass()),
			NAME_None,
			NAME_None,
			TOptional<FLinearColor>(),
			TOptional<int32>(),
			NSLOCTEXT("NexusProcGenEditor", "NCellJunctionBlockerActorPlacement", "Cell Junction Blocker Actor"))));
	}
}

IMPLEMENT_MODULE(FNProcGenEditorModule, NexusProcGenEditor)
