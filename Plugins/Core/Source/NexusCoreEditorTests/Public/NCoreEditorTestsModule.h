// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

/**
 * NEXUS: Core Editor Tests Module
 */
class FNCoreEditorTestsModule final : public IModuleInterface
{
public:
	N_MODULE_BASE(FNCoreEditorTestsModule, "NexusCoreEditorTests")
};
