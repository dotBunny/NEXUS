// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NEditorInputProcessor.h"
#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

/**
 * NEXUS: Tooling Editor Module
 */
// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
class FNToolingEditorModule final : public IModuleInterface
{
public:
	//~IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//End IModuleInterface

	N_IMPLEMENT_MODULE(FNToolingEditorModule, "NexusToolingEditor")

	/** @return the shared editor input processor owned by this module, or nullptr before startup. */
	FNEditorInputProcessor* GetInputProcessor() const
	{
		return InputProcessor.Get();
	}

private:
	/** Module-owned input processor registered with FSlateApplication for the editor's lifetime. */
	TSharedPtr<FNEditorInputProcessor> InputProcessor;

	/** Handle for the tick delegate that re-applies the window icon after the editor initialises. */
	FDelegateHandle WindowIconDelegateHandle;

	/** Replace the editor app icon with the resource at IconPath (SVG or raster). */
	static void ApplyAppIcon(const FString& IconPath);

	/** Replace the platform-level editor window icon with the resource at IconPath (extensionless). */
	void ApplyWindowIcon(const FString& IconPath);

	/** Tick callback that applies the window icon after the main frame exists. */
	void ApplyWindowIconPostEditorTick(float Time) const;

	/** Post-engine-init hook that performs the icon replacements and other one-time setup. */
	void OnPostEngineInit();
};