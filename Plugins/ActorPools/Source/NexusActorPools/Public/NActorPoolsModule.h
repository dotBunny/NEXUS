// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

/**
 * NEXUS: Actor Pools Module
 */
class FNActorPoolsModule final : public IModuleInterface
{
public:	
	virtual void StartupModule() override;
	N_IMPLEMENT_MODULE(FNActorPoolsModule, "NexusActorPools")

private:
	void OnPostEngineInit();
};