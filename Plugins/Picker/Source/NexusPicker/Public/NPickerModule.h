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
	virtual void ShutdownModule() override;
	N_MODULE_BASE(FNPickerModule, "NexusPicker")

private:
	N_MODULE_POST_ENGINE_INIT_STATIC_DELEGATE()
};