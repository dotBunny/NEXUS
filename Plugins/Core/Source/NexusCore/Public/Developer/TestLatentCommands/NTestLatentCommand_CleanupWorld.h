// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#if WITH_TESTS

#include "Misc/AutomationTest.h"
#include "Developer/NTestUtils.h"

class FNTestLatentCommand_CleanupWorld : public IAutomationLatentCommand
{
public:
	virtual ~FNTestLatentCommand_CleanupWorld() override
	{
	}

	virtual bool Update() override
	{
		// Cleanup world
		UWorld* World = FNTestUtils::Environment.World;
		UGameInstance* GameInstance = FNTestUtils::Environment.GameInstance;
	
		World->EndPlay(EEndPlayReason::Quit);
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		GameInstance->MarkAsGarbage();
	
	
		FNTestUtils::Environment.GameInstance = nullptr;
		FNTestUtils::Environment.World = nullptr;
		FNTestUtils::Environment.WorldContext = nullptr;
		return true;
	}
};

#endif // WITH_TESTS
