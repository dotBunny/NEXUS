// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

/**
 * NEXUS: Shared Samples Module
 */
class FNSharedSamplesModule final : public IModuleInterface
{
public:	
	virtual void StartupModule() override;
	N_IMPLEMENT_MODULE(FNSharedSamplesModule, "NexusSharedSamples")
};