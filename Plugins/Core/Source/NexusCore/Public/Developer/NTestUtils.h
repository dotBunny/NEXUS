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
	 * Creates a throwaway UWorld, runs a test body against it, and tears everything down afterward.
	 *
	 * The world is fully booted through InitializeActorsForPlay/BeginPlay so it can host actor
	 * behavior, and is destroyed (along with its temporary UGameInstance) when the test body
	 * returns. Every test that needs a world should go through this helper rather than rolling
	 * its own world lifecycle.
	 *
	 * @param WorldType The EWorldType to create (typically Game or PIE).
	 * @param TestFunctionality Callable that receives the created world and performs the test.
	 * @param bDisableGarbageCollection Suppress GC for the duration of the test body when true.
	 * @remark Useful for straight runs where no ticking is required, but you want a world.
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
		TestGameInstance->MarkAsGarbage();
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
	 * @remark Useful for straight runs where no ticking is required, but you want a world.
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
	
	/**
	 * Captures the current set of filenames in ProjectLogDir matching a wildcard pattern.
	 *
	 * Intended as a "before" baseline for tests that exercise code paths which write files into
	 * the project log directory (snapshot dumps, comparison reports, etc.). Pair with
	 * WaitForNewLogFile() to detect files produced by the test body — the diff between the
	 * baseline and the post-test directory listing identifies the new file unambiguously, even
	 * when prior runs or other tests have left files behind.
	 *
	 * Returned entries are bare filenames (no directory component), matching the format
	 * IFileManager::FindFiles() yields and the format WaitForNewLogFile() expects.
	 *
	 * @param Wildcard Filename glob applied inside ProjectLogDir (e.g. TEXT("NEXUS_Compare_*.txt")).
	 * @return Set of filenames already present in ProjectLogDir that match the wildcard.
	 */
	FORCEINLINE static TSet<FString> SnapshotLogFiles(const TCHAR* Wildcard)
	{
		TArray<FString> Existing;
		IFileManager::Get().FindFiles(Existing, *(FPaths::ProjectLogDir() / Wildcard), true, false);
		return TSet<FString>(Existing);
	}

	/**
	 * Polls ProjectLogDir until a file matching the wildcard appears that is not in PreExisting,
	 * then waits for the writer to finish before returning the path.
	 *
	 * Provides a deterministic synchronisation point for tests whose subject-under-test writes
	 * files asynchronously (Async() tasks, background thread pool, etc.). The poll loop runs at
	 * roughly 25 ms granularity until either a stable file is observed or the timeout elapses, so
	 * the caller does not need access to the producer's task handles or futures.
	 *
	 * The function returns only once the new file is non-empty AND its size has held steady across
	 * one poll interval. The size-stability gate avoids a race where IFileManager::FindFiles() sees
	 * the file the moment the writer creates it (0 bytes, handle still open) — returning that path
	 * would let the caller observe FileSize() == 0 and trip a sharing violation when it tried to
	 * delete the file. By the time the size has stopped changing the writer has serialised the
	 * payload and closed its handle, so the path is safe to read or delete.
	 *
	 * The first new filename encountered is returned as an absolute path (ProjectLogDir joined
	 * with the filename). If multiple new files exist on a given poll iteration the order is
	 * whatever IFileManager::FindFiles() yields — tests that need to disambiguate should narrow
	 * the wildcard.
	 *
	 * @param Wildcard       Filename glob applied inside ProjectLogDir. Must match the wildcard
	 *                       passed to the corresponding SnapshotLogFiles() call.
	 * @param PreExisting    Baseline set returned by SnapshotLogFiles() before the test body ran.
	 * @param TimeoutSeconds Maximum wall-clock time to poll. Default 5s — long enough for typical
	 *                       async file IO under contention without stalling the test suite.
	 * @return Absolute path of the new matching file once its size has stabilised, or an empty
	 *         string on timeout.
	 */
	static FString WaitForNewLogFile(const TCHAR* Wildcard, const TSet<FString>& PreExisting, const double TimeoutSeconds = 5.0);
};
