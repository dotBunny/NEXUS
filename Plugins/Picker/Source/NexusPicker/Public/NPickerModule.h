// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Modules/ModuleInterface.h"
#include "Macros/NModuleMacros.h"

/**
 * NEXUS: Picker Module
 */
class FNPickerModule final : public IModuleInterface
{
	virtual void StartupModule() override;
	void OnPostEngineInit();

	N_IMPLEMENT_MODULE(FNPickerModule, "NexusPicker")
};