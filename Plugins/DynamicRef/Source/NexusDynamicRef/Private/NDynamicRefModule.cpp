// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefModule.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNDynamicRefModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusDynamicRef")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNDynamicRefModule, OnPostEngineInit);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FNDynamicRefModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusDynamicRefSamples")
}

IMPLEMENT_MODULE(FNDynamicRefModule, NexusDynamicRef)
