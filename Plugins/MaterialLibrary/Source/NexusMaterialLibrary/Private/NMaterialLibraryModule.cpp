// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NMaterialLibraryModule.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNMaterialLibraryModule::StartupModule()
{
	
	N_UPDATE_UPLUGIN("NexusMaterialLibrary")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNMaterialLibraryModule, OnPostEngineInit);
}

void FNMaterialLibraryModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusMaterialLibrarySamples")
}

IMPLEMENT_MODULE(FNMaterialLibraryModule, NexusMaterialLibrary)
