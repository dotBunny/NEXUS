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
	/** Delegate returning additional parameters to append to the multiplayer test command line. */
	DECLARE_DELEGATE_RetVal(FString, FMultiplayerTestParametersDelegate);

public:
	/** Invoked when assembling the multiplayer test command line; listeners return a parameter fragment to append. */
	FMultiplayerTestParametersDelegate OnMultiplayerTestParameters;

	/** Broadcast after a multiplayer test session has successfully started. */
	FSimpleMulticastDelegate OnMultiplayerTestStarted;

	/** Broadcast after a multiplayer test session has ended (either user-requested or due to process exit). */
	FSimpleMulticastDelegate OnMultiplayerTestEnded;

	//~IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//End IModuleInterface

	N_MODULE_BASE(FNToolingEditorModule, "NexusToolingEditor")

	/** @return the shared editor input processor owned by this module, or nullptr before startup. */
	FNEditorInputProcessor* GetInputProcessor() const
	{
		return InputProcessor.Get();
	}

private:
	/** Module-owned input processor registered with FSlateApplication for the editor's lifetime. */
	TSharedPtr<FNEditorInputProcessor> InputProcessor;

	/** Handle for the tick delegate that re-applies the window icon after the editor initializes. */
	FDelegateHandle WindowIconDelegateHandle;

	/** Fully-resolved window icon path computed in ApplyWindowIcon and reused by the tick callback. */
	FString WindowIconPath;

	/** Replace the editor app icon with the resource at IconPath (SVG or raster). */
	static void ApplyAppIcon(const FString& IconPath);

	/** Replace the platform-level editor window icon with the resource at IconPath (extensionless). */
	void ApplyWindowIcon(const FString& IconPath);

	/** Tick callback that applies the window icon after the main frame exists. */
	void ApplyWindowIconPostEditorTick(float Time);

	/** Post-engine-init hook that performs the icon replacements and other one-time setup. */
	void OnPostEngineInit();
};