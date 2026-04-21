// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

/**
 * NEXUS: Multiplayer Editor Module
 */
class FNMultiplayerEditorModule final : public IModuleInterface
{
	/** Delegate returning additional parameters to append to the multiplayer test command line. */
	DECLARE_DELEGATE_RetVal(FString, FCollectParametersDelegate);

public:
	/** Invoked when assembling the multiplayer test command line; listeners return a parameter fragment to append. */
	FCollectParametersDelegate OnCollectParameters;
	/** Broadcast after a multiplayer test session has successfully started. */
	FSimpleMulticastDelegate OnMultiplayerTestStarted;
	/** Broadcast after a multiplayer test session has ended (either user-requested or due to process exit). */
	FSimpleMulticastDelegate OnMultiplayerTestEnded;

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	N_IMPLEMENT_MODULE(FNMultiplayerEditorModule, "NexusMultiplayerEditor")

private:
	void OnPostEngineInit();
};