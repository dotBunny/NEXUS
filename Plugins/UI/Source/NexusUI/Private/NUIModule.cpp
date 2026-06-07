// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NUIModule.h"

#include "NCoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNUIModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusUI")
	N_MODULE_POST_ENGINE_INIT_STATIC(FNUIModule::OnPostEngineInit);
}

void FNUIModule::ShutdownModule()
{
	N_MODULE_REMOVE_POST_ENGINE_INIT_DELEGATE()
	IModuleInterface::ShutdownModule();
}

void FNUIModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusUISamples")
}

IMPLEMENT_MODULE(FNUIModule, NexusUI)