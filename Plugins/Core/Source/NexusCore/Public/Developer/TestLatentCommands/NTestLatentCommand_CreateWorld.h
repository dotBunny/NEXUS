// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#if WITH_TESTS

#include "Misc/AutomationTest.h"
#include "Developer/NTestUtils.h"

class FNTestLatentCommand_CreateWorld : public IAutomationLatentCommand
{
public:
	explicit FNTestLatentCommand_CreateWorld(FAutomationTestBase* TestPtr)
		: Test(TestPtr)
	{
	}

	virtual ~FNTestLatentCommand_CreateWorld() override
	{
	}

	virtual bool Update() override
	{
		// Create World
		FNTestUtils::Environment.World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("NTestWorld"));
		UWorld* World = FNTestUtils::Environment.World;
		World->RemoveStreamingLevels(World->GetStreamingLevels());
	
		// Test world exists
		Test->AddErrorIfFalse(World != nullptr, TEXT("World could not be created."));

		// Apply context
		FNTestUtils::Environment.WorldContext = &GEngine->CreateNewWorldContext(EWorldType::Game);
		FWorldContext* WorldContext = FNTestUtils::Environment.WorldContext;
		WorldContext->SetCurrentWorld(World);


		// Start play
		const FURL URL;
		FNTestUtils::Environment.GameInstance = NewObject<UGameInstance>(GEngine);
		World->SetGameInstance(FNTestUtils::Environment.GameInstance);
		World->SetGameMode(URL);
		World->InitializeActorsForPlay(URL);
		World->BeginPlay();
		World->ClearStreamingLevels();
	
		return true;
	}

private:
	FAutomationTestBase* Test;
};

#endif // WITH_TESTS