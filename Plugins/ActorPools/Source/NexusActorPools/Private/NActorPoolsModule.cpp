// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolsModule.h"

#include "NCoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNActorPoolsModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusActorPools")
	N_MODULE_POST_ENGINE_INIT_STATIC(FNActorPoolsModule::OnPostEngineInit);
}

void FNActorPoolsModule::ShutdownModule()
{
	N_MODULE_REMOVE_POST_ENGINE_INIT_DELEGATE()
	IModuleInterface::ShutdownModule();
}

void FNActorPoolsModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusActorPoolsSamples")
}

IMPLEMENT_MODULE(FNActorPoolsModule, NexusActorPools)