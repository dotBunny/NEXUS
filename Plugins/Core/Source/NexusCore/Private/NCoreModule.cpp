// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NCoreModule.h"
#include "NCoreMinimal.h"
#include "NPrimitiveDrawingUtils.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

void FNCoreModule::StartupModule()
{
	N_UPDATE_UPLUGIN("NexusCore")

	// Generate our PDI glyphs as we initialize the module. 
	FNPrimitiveDrawingUtils::GenerateGlyphs();
}

IMPLEMENT_MODULE(FNCoreModule, NexusCore)