// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Math/NSeedGenerator.h"

#include "NProcGenOperationSettings.generated.h"

/**
 * Per-operation settings applied to a single ProcGen generation pass.
 *
 * Lives on the operation itself so each pass can have its own seed and level-instance behaviour
 * independent of the project-wide UNProcGenSettings.
 */
USTRUCT(BlueprintType)
struct NEXUSPROCGEN_API FNProcGenOperationSettings
{
	GENERATED_BODY()

	/** Seed string used to initialize random generation for this pass (via FNSeedGenerator). */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "General")
	FString Seed;

	/** Human-friendly label shown in editor UI and logs when identifying this operation. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "General")
	FText DisplayName;

	/** When true, level instances spawned by this operation are replicated to clients. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Level Instances")
	bool bReplicateLevelInstances = true;

	/** When true, level instance content is streamed in ahead of time rather than on demand. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Level Instances")
	bool bPreLoadLevelInstances = false;

	/** When true, level instances are created for generated cells; when false, only cell proxies exist. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Level Instances")
	bool bCreateLevelInstances = true;

	/** @return Default runtime-generation settings with a freshly generated friendly seed. */
	static FNProcGenOperationSettings GetDefaultSettings()
	{
		FNProcGenOperationSettings Settings;
		Settings.Seed = FNSeedGenerator::RandomFriendlySeed();
		return MoveTemp(Settings);
	}

	/** @return Default editor-preview settings (level instances disabled) with a freshly generated seed. */
	static FNProcGenOperationSettings GetDefaultEditorSettings()
	{
		FNProcGenOperationSettings Settings;
		Settings.bCreateLevelInstances = false;
		Settings.Seed = FNSeedGenerator::RandomFriendlySeed();
		return MoveTemp(Settings);
	}
};
