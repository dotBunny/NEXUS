// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicReferencesEditorModule.h"

#include "NDynamicReferencesComponentCustomization.h"
#include "NDynamicReferenceComponent.h"
#include "NDynamicReferencesEditorStyle.h"
#include "NEditorUtils.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNDynamicReferencesEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNDynamicReferencesEditorModule, OnPostEngineInit);
}

void FNDynamicReferencesEditorModule::ShutdownModule()
{
	// Unregister customizations
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(UNDynamicReferenceComponent::StaticClass()->GetFName());
		PropertyModule.NotifyCustomizationModuleChanged();
	}

	FNDynamicReferencesEditorStyle::Shutdown();
}

void FNDynamicReferencesEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	// Configure Style
	FNDynamicReferencesEditorStyle::Initialize();
	
	// Register Customizations
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(UNDynamicReferenceComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FNDynamicReferencesComponentCustomization::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();
}

IMPLEMENT_MODULE(FNDynamicReferencesEditorModule, NexusDynamicReferencesEditor)
