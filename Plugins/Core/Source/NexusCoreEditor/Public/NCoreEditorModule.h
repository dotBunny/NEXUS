// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorSettings.h"
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
	void OnPostEngineInit();
	
	FNEditorInputProcessor* GetInputProcessor() const
	{
		return InputProcessor.Get();
	}
	
	N_IMPLEMENT_MODULE(FNCoreEditorModule, "NexusCoreEditor")

	static void ApplyAppIcon(const FString& IconPath);
	void ApplyWindowIcon(const FString& IconPath);
	void ApplyWindowIconPostEditorTick(float Time) const;

private:
	TSharedPtr<FNEditorInputProcessor> InputProcessor;
	FDelegateHandle WindowIconDelegateHandle;
};