// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

/**
 * NEXUS: Developer Overlay Module
 */
class FNDeveloperOverlayModule final : public IModuleInterface
{
	virtual void StartupModule() override;
	void OnPostEngineInit();
	
	N_IMPLEMENT_MODULE(FNDeveloperOverlayModule, "NexusDeveloperOverlay")
};