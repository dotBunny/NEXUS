// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

/**
 * NEXUS: Actor Pools Editor Module
 */
class FNActorPoolsEditorModule final : public IModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	void OnPostEngineInit();
	N_IMPLEMENT_MODULE(FNActorPoolsEditorModule, "NexusActorPoolsEditor")
};