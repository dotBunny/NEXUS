// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefsModule.h"

#include "NCoreMinimal.h"
#include "NDynamicRefsMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogNexusDynamicRefs);
N_MODULE_POST_ENGINE_INIT_STATIC_DELEGATE_IMPLEMENTATION(FNDynamicRefsModule)

void FNDynamicRefsModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusDynamicRefs")
	N_MODULE_POST_ENGINE_INIT_STATIC(FNDynamicRefsModule::OnPostEngineInit);
}

void FNDynamicRefsModule::ShutdownModule()
{
	N_MODULE_REMOVE_POST_ENGINE_INIT_DELEGATE()
	IModuleInterface::ShutdownModule();
}

void FNDynamicRefsModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusDynamicRefsSamples")
}

IMPLEMENT_MODULE(FNDynamicRefsModule, NexusDynamicRefs)
