// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NModuleMacros.h"
#include "Modules/ModuleInterface.h"

/**
 * NEXUS: Multiplayer Module
 */
// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
class FNMultiplayerModule final : public IModuleInterface
{
public:	
	virtual void StartupModule() override;
	N_IMPLEMENT_MODULE(FNMultiplayerModule, "NexusMultiplayer")

private:
	FText MultiplayerTestWindowTitle;
	void OnPostEngineInit();
};