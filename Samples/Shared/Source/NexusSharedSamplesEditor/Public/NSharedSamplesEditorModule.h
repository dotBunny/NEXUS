// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

class FPlacementModeID;
/**
 * NEXUS: Shared Samples Editor Module
 */
class FNSharedSamplesEditorModule final : public IModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	void OnPostEngineInit();

	N_IMPLEMENT_MODULE(FNSharedSamplesEditorModule, "NexusSharedSamplesEditor")

private:
	TArray<TOptional<FPlacementModeID>> PlacementActors;	
};