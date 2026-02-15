// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NGuardianModule.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNGuardianModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusGuardian")
}

IMPLEMENT_MODULE(FNGuardianModule, NexusGuardian)
