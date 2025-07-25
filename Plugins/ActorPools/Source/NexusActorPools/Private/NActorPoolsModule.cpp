// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolsModule.h"

#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNActorPoolsModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusActorPools")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNActorPoolsModule, OnPostEngineInit);
}

void FNActorPoolsModule::OnPostEngineInit()
{
	if (IsModuleInitialized()) return;
	
	N_UPDATE_UPLUGIN("NexusActorPoolsSamples")

	bIsModuleInitialized = true;
}

IMPLEMENT_MODULE(FNActorPoolsModule, NexusActorPools)