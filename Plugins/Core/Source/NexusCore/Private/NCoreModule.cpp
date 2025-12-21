// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCoreModule.h"
#include "NCoreMinimal.h"
#include "NPrimitiveFont.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNCoreModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusCore")

	UE_LOG(LogNexusCore, Log, TEXT("NEXUS Version %i (%i.%i.%i)"), N_VERSION_NUMBER, N_VERSION_MAJOR, N_VERSION_MINOR, N_VERSION_PATCH);
	
	// Initialize our font glyphs
	FNPrimitiveFont::Initialize();
}

IMPLEMENT_MODULE(FNCoreModule, NexusCore)