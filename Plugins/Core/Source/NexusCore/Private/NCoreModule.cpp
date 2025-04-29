// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCoreModule.h"
#include "NCoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNCoreModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusCore")
}

IMPLEMENT_MODULE(FNCoreModule, NexusCore)