// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

/**
 * NEXUS: Procedural Generation Module
 */
class FNWorldAssemblyModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	N_MODULE_BASE(FNWorldAssemblyModule, "NexusWorldAssembly")

private:
	void OnPostEngineInit();
	FDelegateHandle PostWorldCleanupHandle;
};