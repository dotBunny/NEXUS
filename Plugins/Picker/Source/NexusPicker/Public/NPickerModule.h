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
public:
	virtual void StartupModule() override;
	N_IMPLEMENT_MODULE(FNPickerModule, "NexusPicker")

private:
	void OnPostEngineInit();
};