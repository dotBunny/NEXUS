// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NObjectSnapshotUtils.h"
#include "Misc/LowLevelTestAdapter.h"

class NEXUSCORE_API FNTestUtils
{
public:
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
		World->InitializeActorsForPlay(URL);
		World->BeginPlay();

		// Execute Test

		// ReSharper disable once CppTooWideScope
		TOptional<FGCScopeGuard> GCGuard;
		if (bDisableGarbageCollection)
		{
			GCGuard.Emplace();
		}

		TestFunctionality(World);
		

		// Cleanup world
		World->EndPlay(EEndPlayReason::Quit);
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(bInformEngineOfWorld);
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
