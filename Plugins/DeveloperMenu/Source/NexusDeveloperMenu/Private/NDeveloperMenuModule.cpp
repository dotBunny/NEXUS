// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDeveloperMenuModule.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNDeveloperMenuModule::StartupModule()
{
	
	N_UPDATE_UPLUGIN("NexusDeveloperMenu")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNDeveloperMenuModule, OnPostEngineInit);
}

void FNDeveloperMenuModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusDeveloperMenuSamples")
}

IMPLEMENT_MODULE(FNDeveloperMenuModule, NexusDeveloperMenu)
