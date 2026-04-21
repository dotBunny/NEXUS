// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

/**
 * NEXUS: User Interface Module
 */
// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
class FNUIModule final : public IModuleInterface
{
public:
	//~IModuleInterface
	virtual void StartupModule() override;
	//End IModuleInterface

	N_IMPLEMENT_MODULE(FNUIModule, "NexusUI")

private:
	/** Post-engine-init hook used to register things that depend on slate/umg being fully live. */
	void OnPostEngineInit();
};