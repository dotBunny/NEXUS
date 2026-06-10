// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NBlockoutModule.h"

#include "NCoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

N_MODULE_POST_ENGINE_INIT_STATIC_DELEGATE_IMPLEMENTATION(FNBlockoutModule)

void FNBlockoutModule::StartupModule()
{
	
	N_UPDATE_UPLUGIN("NexusBlockout")
	N_MODULE_POST_ENGINE_INIT_STATIC(FNBlockoutModule::OnPostEngineInit);
}

void FNBlockoutModule::ShutdownModule()
{
	N_MODULE_REMOVE_POST_ENGINE_INIT_DELEGATE()
	IModuleInterface::ShutdownModule();
}

void FNBlockoutModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusBlockoutSamples")
}

IMPLEMENT_MODULE(FNBlockoutModule, NexusBlockout)
