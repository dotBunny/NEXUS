// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicReferencesModule.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNDynamicReferencesModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusDynamicReferences")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNDynamicReferencesModule, OnPostEngineInit);
}

void FNDynamicReferencesModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusDynamicReferencesSamples")
}

IMPLEMENT_MODULE(FNDynamicReferencesModule, NexusDynamicReferences)
