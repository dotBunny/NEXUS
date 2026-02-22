// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorInputProcessor.h"
#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

/**
 * NEXUS: Tools Editor Module
 */

// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
class FNToolingEditorModule final : public IModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	void OnPostEngineInit();
	
	static void ApplyAppIcon(const FString& IconPath);
	void ApplyWindowIcon(const FString& IconPath);
	void ApplyWindowIconPostEditorTick(float Time) const;

public:
	FNEditorInputProcessor* GetInputProcessor() const
	{
		return InputProcessor.Get();
	}

	N_IMPLEMENT_MODULE(FNToolingEditorModule, "NexusToolingEditor")
	
private:
	TSharedPtr<FNEditorInputProcessor> InputProcessor;
	FDelegateHandle WindowIconDelegateHandle;
};