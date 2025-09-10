// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NBlockoutModule.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNBlockoutModule::StartupModule()
{
	
	N_UPDATE_UPLUGIN("NexusBlockout")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNBlockoutModule, OnPostEngineInit);
}

void FNBlockoutModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusBlockoutSamples")
}

IMPLEMENT_MODULE(FNBlockoutModule, NexusBlockout)
