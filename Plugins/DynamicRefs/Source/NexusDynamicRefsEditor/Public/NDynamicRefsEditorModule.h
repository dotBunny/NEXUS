// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

/**
 * NEXUS: DynamicRefs Editor Module
 */
class FNDynamicRefsEditorModule final : public IModuleInterface
{
public:	
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	N_IMPLEMENT_MODULE(FNDynamicRefsEditorModule, "NexusDynamicRefsEditor")
	
private:	
	void OnPostEngineInit();
};



