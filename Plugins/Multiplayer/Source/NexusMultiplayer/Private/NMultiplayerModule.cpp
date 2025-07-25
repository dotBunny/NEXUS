// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NMultiplayerModule.h"

#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNMultiplayerModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusMultiplayer")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNMultiplayerModule, OnPostEngineInit);
}

void FNMultiplayerModule::OnPostEngineInit()
{
	if (IsModuleInitialized()) return;
	N_UPDATE_UPLUGIN("NexusMultiplayerSamples")
	bIsModuleInitialized = true;
}

IMPLEMENT_MODULE(FNMultiplayerModule, NexusMultiplayer)