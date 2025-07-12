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
	virtual void StartupModule() override;
	void OnPostEngineInit();

	N_IMPLEMENT_MODULE(FNUIModule, "NexusUI")
};