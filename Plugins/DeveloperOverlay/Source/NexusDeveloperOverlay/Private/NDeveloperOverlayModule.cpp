// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDeveloperOverlayModule.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNDeveloperOverlayModule::StartupModule()
{
	
	N_UPDATE_UPLUGIN("NexusDeveloperOverlay")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNDeveloperOverlayModule, OnPostEngineInit);
}

void FNDeveloperOverlayModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusDeveloperOverlaySamples")
}

IMPLEMENT_MODULE(FNDeveloperOverlayModule, NexusDeveloperOverlay)
