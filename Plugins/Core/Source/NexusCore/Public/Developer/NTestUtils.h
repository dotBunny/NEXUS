// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NObjectSnapshotUtils.h"
#include "Misc/LowLevelTestAdapter.h"

/**
 * Helpers that standardise how NEXUS tests set up and tear down worlds and measure performance.
 *
 * These utilities are only meaningful inside the Low-Level Test framework and therefore depend on
 * its macros (REQUIRE_MESSAGE, ADD_ERROR). All methods are designed to be safely re-entrant — every
 * world is disposed before the call returns.
 */
class NEXUSCORE_API FNTestUtils
{
public:
	/**
	 * Prepares global state so a performance test can produce comparable, low-noise measurements.
	 * @note Initializes stack walking, forces a GC and flushes the log/visual-log streams.
	 */
	FORCEINLINE static void PrePerformanceTest()
	{
		// Ensure that stack walking is initialized prior to performance testing to avoid library loading
		FPlatformStackWalk::InitStackWalking();

		// Force garbage collection to ensure a clean slate for performance testing
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

		// Force the logs to be written to disk prior to the operation
		GLog->Flush();
		FVisualLogger::Get().Flush();
	}

	/** Matching teardown to PrePerformanceTest(); forces a GC so the next test starts clean. */
	FORCEINLINE static void PostPerformanceTest()
	{
		// Force garbage collection to ensure the next test is already prepared / faster
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	}

	/**
	 * Creates a throwaway UWorld, runs a test body against it, and tears everything down afterwards.
	 *
	 * The world is fully booted through InitializeActorsForPlay/BeginPlay so it can host actor
	 * behaviour, and is destroyed (along with its temporary UGameInstance) when the test body
	 * returns. Every test that needs a world should go through this helper rather than rolling
	 * its own world lifecycle.
	 *
	 * @param WorldType The EWorldType to create (typically Game or PIE).
	 * @param TestFunctionality Callable that receives the created world and performs the test.
	 * @param bDisableGarbageCollection Suppress GC for the duration of the test body when true.
	 */
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
			{
				TestFunctionality(World);
			}
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
	
	/**
	 * Runs a world test and asserts that no UObjects leaked across the scope.
	 *
	 * Captures a baseline UObject snapshot, delegates to WorldTest(), and then compares a post-test
	 * snapshot against the baseline. Any newly added objects are reported via ADD_ERROR and fail
	 * the test. When bShouldGarbageCollect is true an additional GC pass runs between the test body
	 * and the comparison so transient objects have a chance to be collected.
	 *
	 * @param WorldType The EWorldType to create for the world fixture.
	 * @param TestFunctionality Callable that receives the created world and performs the test.
	 * @param bShouldGarbageCollect Run GC between the test body and the leak check when true.
	 */
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
			
			// Always cleanup after the check
			CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		}
	}
};
