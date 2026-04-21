// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

/**
 * NEXUS: Procedural Generation Module
 */
// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
class FNProcGenModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	N_IMPLEMENT_MODULE(FNProcGenModule, "NexusProcGen")

private:
	void OnPostEngineInit();
};