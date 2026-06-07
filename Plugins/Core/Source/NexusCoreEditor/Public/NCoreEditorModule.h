// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

class FNEditorInputProcessor;

/**
 * NEXUS: Core Editor Module
 */
class FNCoreEditorModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	N_MODULE_BASE(FNCoreEditorModule, "NexusCoreEditor")

private:
	N_MODULE_POST_ENGINE_INIT_STATIC_DELEGATE()
};