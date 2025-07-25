// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NUIModule.h"

#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNUIModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusUI")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNUIModule, OnPostEngineInit);
}

void FNUIModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusUISamples")
}

IMPLEMENT_MODULE(FNUIModule, NexusUI)