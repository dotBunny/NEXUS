// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

class FPlacementModeID;
/**
 * NEXUS: Actor Pools Editor Module
 */
class FNActorPoolsEditorModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	N_IMPLEMENT_MODULE(FNActorPoolsEditorModule, "NexusActorPoolsEditor")

private:
	/** Registered placement-mode entries, tracked so they can be unregistered on shutdown. */
	TArray<TOptional<FPlacementModeID>> PlacementActors;
	void OnPostEngineInit();
};