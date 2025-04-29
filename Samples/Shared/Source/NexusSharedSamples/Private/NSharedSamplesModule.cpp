// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NSharedSamplesModule.h"

#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNSharedSamplesModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusSharedSamples")
}

IMPLEMENT_MODULE(FNSharedSamplesModule, NexusSharedSamples)
