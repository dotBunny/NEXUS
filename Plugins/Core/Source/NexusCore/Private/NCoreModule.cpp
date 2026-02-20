// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCoreModule.h"
#include "NCoreMinimal.h"
#include "Developer/NPrimitiveFont.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNCoreModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusCore")

	UE_LOG(LogNexusCore, Log, TEXT("NEXUS Version %i (%i.%i.%i)"), NEXUS::Version::Number, NEXUS::Version::Major, NEXUS::Version::Minor, NEXUS::Version::Patch);
	
	// Initialize our font glyphs
	FNPrimitiveFont::Initialize();
}

IMPLEMENT_MODULE(FNCoreModule, NexusCore)