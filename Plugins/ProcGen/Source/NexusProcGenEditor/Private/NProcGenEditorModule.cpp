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
#include "Customizations/NCellRootComponentCustomization.h"
#include "Cell/NCellJunctionComponent.h"
#include "Visualizers/NCellJunctionComponentVisualizer.h"
#include "Cell/NCellProxy.h"
#include "Customizations/NCellProxyCustomization.h"
#include "Cell/NCellRootComponent.h"
#include "NProcGenComponent.h"
#include "Visualizers/NCellRootComponentVisualizer.h"
#include "NProcGenEditorCommands.h"
#include "NProcGenEditorToolMenu.h"
#include "NProcGenEdMode.h"
#include "NProcGenVolume.h"
#include "UnrealEdGlobals.h"
#include "Customizations/NProcGenComponentCustomization.h"
#include "Editor/UnrealEdEngine.h"

void FNProcGenEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNProcGenEditorModule, OnPostEngineInit);
}

void FNProcGenEditorModule::ShutdownModule()
{
	FEditorModeRegistry::Get().UnregisterMode(FNProcGenEdMode::Identifier);

	//GUnrealEd->UnregisterComponentVisualizer(UNCellRootComponent::StaticClass()->GetFName());
	//GUnrealEd->UnregisterComponentVisualizer(UNCellPinComponent::StaticClass()->GetFName());

	// Unregister customizations
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

		PropertyModule.UnregisterCustomClassLayout(UNCellJunctionComponent::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UNCellRootComponent::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(ANCellActor::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(ANCellProxy::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UNProcGenComponent::StaticClass()->GetFName());

		PropertyModule.NotifyCustomizationModuleChanged();
	}
	
	FNProcGenEditorStyle::Shutdown();
}

void FNProcGenEditorModule::OnPostEngineInit()
{
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
	const TSharedPtr<FComponentVisualizer> RootComponentVisualizer = MakeShareable(new FNCellRootComponentVisualizer());
	GUnrealEd->RegisterComponentVisualizer(UNCellRootComponent::StaticClass()->GetFName(), RootComponentVisualizer);
	RootComponentVisualizer->OnRegister();
	
	const TSharedPtr<FComponentVisualizer> PinComponentVisualizer = MakeShareable(new FNCellJunctionComponentVisualizer());
	GUnrealEd->RegisterComponentVisualizer(UNCellJunctionComponent::StaticClass()->GetFName(), PinComponentVisualizer);
	PinComponentVisualizer->OnRegister();

	// Register Customizations
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	PropertyModule.RegisterCustomClassLayout(ANCellProxy::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FNCellProxyCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(UNCellRootComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FNCellRootComponentCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(UNProcGenComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FNProcGenComponentCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

IMPLEMENT_MODULE(FNProcGenEditorModule, NexusProcGenEditor)
