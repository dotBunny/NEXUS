// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDeveloperConsoleModule.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNDeveloperConsoleModule::StartupModule()
{
	
	N_UPDATE_UPLUGIN("NexusDeveloperConsole")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNDeveloperConsoleModule, OnPostEngineInit);
}

void FNDeveloperConsoleModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusDeveloperConsoleSamples")
}

IMPLEMENT_MODULE(FNDeveloperConsoleModule, NexusDeveloperConsole)
