// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NMultiplayerModule.h"

#include "NMultiplayerUtils.h"
#include "HAL/PlatformOutputDevices.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Modules/ModuleManager.h"

void FNMultiplayerModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusMultiplayer")
}


IMPLEMENT_MODULE(FNMultiplayerModule, NexusMultiplayer)
