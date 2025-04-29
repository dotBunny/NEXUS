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
	virtual void StartupModule() override;

	N_IMPLEMENT_MODULE(FNCoreModule, "NexusCore")
};