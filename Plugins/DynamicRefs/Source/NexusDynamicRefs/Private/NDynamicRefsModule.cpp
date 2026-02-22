// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefsModule.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNDynamicRefsModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusDynamicRefs")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNDynamicRefsModule, OnPostEngineInit);
}
 
// ReSharper disable once CppMemberFunctionMayBeStatic
void FNDynamicRefsModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusDynamicRefsSamples")
}

IMPLEMENT_MODULE(FNDynamicRefsModule, NexusDynamicRefs)
