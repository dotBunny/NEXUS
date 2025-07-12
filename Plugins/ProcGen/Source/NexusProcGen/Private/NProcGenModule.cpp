// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenModule.h"

#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNProcGenModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusProcGen")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNProcGenModule, OnPostEngineInit);
}

void FNProcGenModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusProcGenSamples")
}

IMPLEMENT_MODULE(FNProcGenModule, NexusProcGen)