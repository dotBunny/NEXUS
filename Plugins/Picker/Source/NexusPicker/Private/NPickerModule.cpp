// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NPickerModule.h"

#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNPickerModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusPicker")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNPickerModule, OnPostEngineInit);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FNPickerModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusPickerSamples")
}

IMPLEMENT_MODULE(FNPickerModule, NexusPicker)