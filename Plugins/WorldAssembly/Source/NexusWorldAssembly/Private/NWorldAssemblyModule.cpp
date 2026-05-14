// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyModule.h"

#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNWorldAssemblyModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusWorldAssembly")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNWorldAssemblyModule, OnPostEngineInit);
}

void FNWorldAssemblyModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusWorldAssemblySamples")
}

IMPLEMENT_MODULE(FNWorldAssemblyModule, NexusWorldAssembly)