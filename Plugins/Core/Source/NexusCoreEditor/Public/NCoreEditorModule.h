// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

class FNEditorInputProcessor;

/**
 * NEXUS: Core Editor Module
 * @remark Loads during PostEngineInit
 */
class FNCoreEditorModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	void OnPostEngineInit();
	FNEditorInputProcessor* GetInputProcessor() const
	{
		return InputProcessor.Get();
	}
	
	N_IMPLEMENT_MODULE(FNCoreEditorModule, "NexusCoreEditor")

private:
	UPROPERTY()
	TSharedPtr<FNEditorInputProcessor> InputProcessor;
};