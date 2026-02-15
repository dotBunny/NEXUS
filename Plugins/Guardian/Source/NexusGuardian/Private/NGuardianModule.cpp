// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NGuardianModule.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNGuardianModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusGuardian")
	N_IMPLEMENT_MODULE_POST_ENGINE_INIT(FNGuardianModule, OnPostEngineInit);
}
 
// ReSharper disable once CppMemberFunctionMayBeStatic
void FNGuardianModule::OnPostEngineInit()
{
	N_UPDATE_UPLUGIN("NexusGuardianSamples")
}

IMPLEMENT_MODULE(FNGuardianModule, NexusGuardian)
