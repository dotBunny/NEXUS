// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Developer/NObjectSnapshotUtils.h"
#include "HAL/IConsoleManager.h"

namespace NConsoleCommands::Developer
{
	static FAutoConsoleCommand SnapshotToDisk(TEXT("N.Developer.Snapshot"),
		TEXT("Take a snapshot of the currently known objects and save to the projects log folder."),
		FConsoleCommandDelegate::CreateStatic(&FNObjectSnapshotUtils::SnapshotToDisk));
	
	static FAutoConsoleCommand CacheSnapshot(TEXT("N.Developer.CacheSnapshot"),
		TEXT("Caches a snapshot to be compared against manually via N.Developer.CompareSnapshot"),
		FConsoleCommandDelegate::CreateStatic(&FNObjectSnapshotUtils::CacheSnapshot));
	
	static FAutoConsoleCommand CompareSnapshot(TEXT("N.Developer.CompareSnapshot"),
		TEXT("Compares the a current snapshot to the previously cached one and outputs results to disk."),
		FConsoleCommandDelegate::CreateStatic(&FNObjectSnapshotUtils::CompareSnapshotToDisk));

	static FAutoConsoleCommand ClearCachedSnapshot(TEXT("N.Developer.ClearCachedSnapshot"),
		TEXT("Clears the cached snapshot."),
		FConsoleCommandDelegate::CreateStatic(&FNObjectSnapshotUtils::ClearCachedSnapshot));
}