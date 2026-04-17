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
	DECLARE_DELEGATE_RetVal(FString, FCollectParametersDelegate);

public:
	FCollectParametersDelegate OnCollectParameters;
	FSimpleMulticastDelegate OnMultiplayerTestStarted;
	FSimpleMulticastDelegate OnMultiplayerTestEnded;
	
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	N_IMPLEMENT_MODULE(FNMultiplayerEditorModule, "NexusMultiplayerEditor")

private:
	void OnPostEngineInit();
};