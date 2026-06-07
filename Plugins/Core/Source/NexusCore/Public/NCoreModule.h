// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Modules/ModuleInterface.h"
#include "Macros/NModuleMacros.h"

/**
 * NEXUS: Core Module
 */
class FNCoreModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	N_MODULE_BASE(FNCoreModule, "NexusCore")

#if WITH_EDITOR	
private:
	/** Cached localized title used by the multiplayer test window. */
	FText MultiplayerTestWindowTitle;
	void OnPostEngineInit();	
#endif // WITH_EDITOR	
};