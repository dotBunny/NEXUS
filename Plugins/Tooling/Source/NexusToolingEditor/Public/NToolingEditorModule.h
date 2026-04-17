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
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	N_IMPLEMENT_MODULE(FNToolingEditorModule, "NexusToolingEditor")
	FNEditorInputProcessor* GetInputProcessor() const
	{
		return InputProcessor.Get();
	}

private:	
	TSharedPtr<FNEditorInputProcessor> InputProcessor;
	FDelegateHandle WindowIconDelegateHandle;

	static void ApplyAppIcon(const FString& IconPath);
	void ApplyWindowIcon(const FString& IconPath);
	void ApplyWindowIconPostEditorTick(float Time) const;
	void OnPostEngineInit();
};