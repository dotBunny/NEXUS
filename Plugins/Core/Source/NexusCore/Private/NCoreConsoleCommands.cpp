// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Async/Async.h"
#include "Developer/NObjectSnapshotUtils.h"
#include "HAL/IConsoleManager.h"

namespace NEXUS::Core::ConsoleCommands
{
	// Console commands can dispatch from the console thread; the snapshot pipeline walks GUObjectArray
	// and mutates static state, so each handler bounces to the game thread before doing any work.

	static FAutoConsoleCommand SnapshotToDisk(TEXT("N.Developer.Snapshot"),
		TEXT("Take a snapshot of the currently known objects and save to the projects log folder."),
		FConsoleCommandDelegate::CreateLambda([]
		{
			AsyncTask(ENamedThreads::GameThread, []{ FNObjectSnapshotUtils::SnapshotToDisk(); });
		}));

	static FAutoConsoleCommand CacheSnapshot(TEXT("N.Developer.CacheSnapshot"),
		TEXT("Caches a snapshot to be compared against manually via N.Developer.CompareSnapshot"),
		FConsoleCommandDelegate::CreateLambda([]
		{
			AsyncTask(ENamedThreads::GameThread, []{ FNObjectSnapshotUtils::CacheSnapshot(); });
		}));

	static FAutoConsoleCommand CompareSnapshot(TEXT("N.Developer.CompareSnapshot"),
		TEXT("Compares the a current snapshot to the previously cached one and outputs results to disk."),
		FConsoleCommandDelegate::CreateLambda([]
		{
			AsyncTask(ENamedThreads::GameThread, []{ FNObjectSnapshotUtils::CompareSnapshotToDisk(); });
		}));

	static FAutoConsoleCommand ClearCachedSnapshot(TEXT("N.Developer.ClearCachedSnapshot"),
		TEXT("Clears the cached snapshot."),
		FConsoleCommandDelegate::CreateLambda([]
		{
			AsyncTask(ENamedThreads::GameThread, []{ FNObjectSnapshotUtils::ClearCachedSnapshot(); });
		}));
}
