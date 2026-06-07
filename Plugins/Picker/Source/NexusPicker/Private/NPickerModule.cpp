// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NPickerModule.h"

#include "NCoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNPickerModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusPicker")
	N_MODULE_POST_ENGINE_INIT_STATIC(FNPickerModule::OnPostEngineInit);
}

void FNPickerModule::ShutdownModule()
{
	N_MODULE_REMOVE_POST_ENGINE_INIT_DELEGATE()
	IModuleInterface::ShutdownModule();
}

void FNPickerModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusPickerSamples")
}

IMPLEMENT_MODULE(FNPickerModule, NexusPicker)