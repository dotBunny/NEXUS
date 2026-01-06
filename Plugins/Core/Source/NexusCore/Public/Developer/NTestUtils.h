// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NObjectSnapshotUtils.h"
#include "Misc/LowLevelTestAdapter.h"

class NEXUSCORE_API FNTestUtils
{
public:
	
	FORCEINLINE static void PrePerformanceTest()
	{
		// Ensure that stack walking is initialized prior to performance testing to avoid library loading
		FPlatformStackWalk::InitStackWalking();
	
		// Force garbage collection to ensure a clean slate for performance testing
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		
		// Force the logs to be written to disk prior to the operation
		GLog->Flush();
	}
	
	FORCEINLINE static void PostPerformanceTest()
	{
		// Force garbage collection to ensure the next test is already prepared / faster
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	}
	
	FORCEINLINE static void WorldTest(const EWorldType::Type WorldType, const TFunctionRef<void(UWorld* World)>& TestFunctionality, const bool bDisableGarbageCollection = false)
	{
		// Create World
		constexpr bool bInformEngineOfWorld = false;
		UWorld* World = UWorld::CreateWorld(WorldType, bInformEngineOfWorld, TEXT("NTestWorld"));

		// Test world exists
		REQUIRE_MESSAGE(TEXT("World could not be created."), World != nullptr);

		FWorldContext& WorldContext = GEngine->CreateNewWorldContext(WorldType);
		WorldContext.SetCurrentWorld(World);

		// Start play
		const FURL URL;
		UGameInstance* TestGameInstance = NewObject<UGameInstance>(GEngine);
		World->SetGameInstance(TestGameInstance);
		World->SetGameMode(URL);
		World->InitializeActorsForPlay(URL);
		World->BeginPlay();

		// Execute Test

		// ReSharper disable once CppTooWideScope
		if (bDisableGarbageCollection)
		{
			FGCScopeGuard Guard;
			TestFunctionality(World);
		}
		else
		{
			TestFunctionality(World);
		}

		// Cleanup world
		World->EndPlay(EEndPlayReason::Quit);
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(bInformEngineOfWorld);
		TestGameInstance->ConditionalBeginDestroy();
	}
	
	FORCEINLINE static void WorldTestChecked(const EWorldType::Type WorldType, const TFunctionRef<void(UWorld* World)>& TestFunctionality, const bool bShouldGarbageCollect = true)
	{
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		const FNObjectSnapshot PreSnapshot = FNObjectSnapshotUtils::Snapshot();
		
		WorldTest(WorldType, TestFunctionality);
		const FNObjectSnapshot PostSnapshot = FNObjectSnapshotUtils::Snapshot();
		
		if (bShouldGarbageCollect)
		{
			CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

			const FNObjectSnapshot GCSnapshot = FNObjectSnapshotUtils::Snapshot();
			if (FNObjectSnapshotDiff GCDiff = FNObjectSnapshotUtils::Diff(PreSnapshot, GCSnapshot, true);
				GCDiff.AddedCount > 0)
			{
				for (int32 i = 0; i < GCDiff.AddedCount; i++)
				{
					ADD_ERROR(FString::Printf(TEXT("Leaked %s"), *GCDiff.Added[i].ToString()));
				}
			}
		}
		else
		{
			if (FNObjectSnapshotDiff Diff = FNObjectSnapshotUtils::Diff(PreSnapshot, PostSnapshot, true);
				Diff.AddedCount > 0)
			{
				for (int32 i = 0; i < Diff.AddedCount; i++)
				{
					ADD_ERROR(FString::Printf(TEXT("Leaked %s"), *Diff.Added[i].ToString()));
				}
			}
		}
	}
};
