// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSharedSamplesEditorModule.h"

#include "NEditorUtils.h"
#include "Customizations/NSamplesDisplayActorCustomization.h"
#include "NSamplesDisplayActor.h"
#include "NSamplesLevelActor.h"
#include "NSharedSamplesEditorStyle.h"
#include "Customizations/NSamplesLevelActorCustomization.h"

#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "Macros/NModuleMacros.h"

void FNSharedSamplesEditorModule::StartupModule()
{
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNSharedSamplesEditorModule, OnPostEngineInit);
}

void FNSharedSamplesEditorModule::ShutdownModule()
{
	// Unregister customizations
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

		PropertyModule.UnregisterCustomClassLayout(ANSamplesDisplayActor::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(ANSamplesLevelActor::StaticClass()->GetFName());	

		PropertyModule.NotifyCustomizationModuleChanged();
	}

	FNSharedSamplesEditorStyle::Shutdown();
}

void FNSharedSamplesEditorModule::OnPostEngineInit()
{
	if (!FNEditorUtils::IsUserControlled()) return;
	
	// Configure Style
	FNSharedSamplesEditorStyle::Initialize();
	
	// Register Customizations
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	PropertyModule.RegisterCustomClassLayout(ANSamplesDisplayActor::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FNSamplesDisplayActorCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(ANSamplesLevelActor::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FNSamplesLevelActorCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

IMPLEMENT_MODULE(FNSharedSamplesEditorModule, NexusSharedSamplesEditor)
