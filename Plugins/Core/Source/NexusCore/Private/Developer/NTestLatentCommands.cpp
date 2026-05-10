// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Developer/NTestLatentCommands.h"

FNTestEnvironment FNTestLatentCommands::Environment;

bool FNTestLatentCommand_FrameWait::Update() 
{
	static int32 ElapsedFrames = 0;
	if (ElapsedFrames < TotalFramesToWait)
	{
		ElapsedFrames++;
		return false;
	}
	ElapsedFrames = 0;
	return true;
}




bool FNTestLatentCommand_CreateWorld::Update()
{
	// Create World
	FNTestLatentCommands::Environment.World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("NTestWorld"));
	UWorld* World = FNTestLatentCommands::Environment.World;
	World->RemoveStreamingLevels(World->GetStreamingLevels());
	
	// Test world exists
	Test->AddErrorIfFalse(World != nullptr, TEXT("World could not be created."));

	// Apply context
	FNTestLatentCommands::Environment.WorldContext = &GEngine->CreateNewWorldContext(EWorldType::Game);
	FWorldContext* WorldContext = FNTestLatentCommands::Environment.WorldContext;
	WorldContext->SetCurrentWorld(World);


	// Start play
	const FURL URL;
	FNTestLatentCommands::Environment.GameInstance = NewObject<UGameInstance>(GEngine);
	World->SetGameInstance(FNTestLatentCommands::Environment.GameInstance);
	World->SetGameMode(URL);
	World->InitializeActorsForPlay(URL);
	World->BeginPlay();
	World->ClearStreamingLevels();
	
	return true;
}


bool FNTestLatentCommand_CleanupWorld::Update()
{
	// Cleanup world
	UWorld* World = FNTestLatentCommands::Environment.World;
	UGameInstance* GameInstance = FNTestLatentCommands::Environment.GameInstance;
	
	World->EndPlay(EEndPlayReason::Quit);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	GameInstance->MarkAsGarbage();
	
	
	FNTestLatentCommands::Environment.GameInstance = nullptr;
	FNTestLatentCommands::Environment.World = nullptr;
	FNTestLatentCommands::Environment.WorldContext = nullptr;
	return true;
}


bool FNTestLatentCommand_PrePerformanceTest::Update()
{
	// Ensure that stack walking is initialized prior to performance testing to avoid library loading, this happens once.
	FNTestLatentCommands::Environment.InitializeStackWalking();
	
	// Force garbage collection to ensure a clean slate for performance testing
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

	// Force the logs to be written to disk prior to the operation
	GLog->Flush();
	FVisualLogger::Get().Flush();
		
	// Ensure everything has been loaded
	IStreamingManager::Get().StreamAllResources();
	return true;
}


bool FNTestLatentCommand_PostPerformanceTest::Update()
{
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	return true;
}

bool FNTestLatentCommand::Update()
{
	TestFunctionality();
	return true;
}

bool FNTestLatentCommand_WorldTest::Update()
{
	if (bDisableGarbageCollection)
	{
		FGCScopeGuard Guard;
		{
			TestFunctionality(FNTestLatentCommands::Environment.World);
		}
	}
	else
	{
		TestFunctionality(FNTestLatentCommands::Environment.World);
	}
	return true;
}