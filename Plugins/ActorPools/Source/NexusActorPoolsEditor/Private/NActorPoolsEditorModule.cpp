// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolsEditorModule.h"
#include "ComponentVisualizer.h"
#include "NActorPoolsEditorStyle.h"
#include "NActorPoolSpawnerComponent.h"
#include "NEditorUtils.h"
#include "UnrealEdGlobals.h"
#include "Customizations/NActorPoolSpawnerComponentCustomization.h"
#include "Editor/UnrealEdEngine.h"
#include "Macros/NModuleMacros.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "Visualizers/NActorPoolSpawnerComponentVisualizer.h"

void FNActorPoolsEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNActorPoolsEditorModule, OnPostEngineInit);
}

void FNActorPoolsEditorModule::ShutdownModule()
{

	//GUnrealEd->UnregisterComponentVisualizer(UNActorPoolSpawnerComponent::StaticClass()->GetFName());

	// Unregister customizations
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

		PropertyModule.UnregisterCustomClassLayout(UNActorPoolSpawnerComponent::StaticClass()->GetFName());

		PropertyModule.NotifyCustomizationModuleChanged();
	}
	
	FNActorPoolsEditorStyle::Shutdown();
}

void FNActorPoolsEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	FNActorPoolsEditorStyle::Initialize();

	// Visualizers
	if (GUnrealEd)
	{
		const TSharedPtr<FComponentVisualizer> ActorPoolSpawnerComponentVisualizer = MakeShareable(new FNActorPoolSpawnerComponentVisualizer());
		GUnrealEd->RegisterComponentVisualizer(UNActorPoolSpawnerComponent::StaticClass()->GetFName(), ActorPoolSpawnerComponentVisualizer);
		ActorPoolSpawnerComponentVisualizer->OnRegister();
	}
	
	// Register Customizations
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomClassLayout(UNActorPoolSpawnerComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FNActorPoolSpawnerComponentCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

IMPLEMENT_MODULE(FNActorPoolsEditorModule, NexusActorPoolsEditor)