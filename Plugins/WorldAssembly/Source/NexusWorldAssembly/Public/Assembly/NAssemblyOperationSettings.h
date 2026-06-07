// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "GameplayTagContainer.h"
#include "NWorldAssemblySettings.h"
#include "Collections/NGameplayTagCounter.h"
#include "Math/NSeedGenerator.h"

#include "NAssemblyOperationSettings.generated.h"

/**
 * Per-operation settings applied to a single World Assembly generation pass.
 *
 * Lives on the operation itself so each pass can have its own seed and level-instance behavior
 * independent of the project-wide UNWorldAssemblySettings.
 */
USTRUCT(BlueprintType)
struct NEXUSWORLDASSEMBLY_API FNAssemblyOperationSettings
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
	
	/** Maximum absolute degree deviation (+/-) from a cell's DirectionConstraint heading that still permits placement. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Directions")
	float AssemblyDirectionTolerance = 15.f;
	
	/** Context tags applied to the operation and propagated into the generation context. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Tagging")
	FGameplayTagContainer ContextTags;

	/** Per-tag usage counters seeded into the operation (e.g. to use for conditional logic during cell filtering). */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Tagging")
	FNGameplayTagCounter TagCounters;

	/** Per-frame time budget for spawning cell proxies during this operation, in milliseconds. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Spawning")
	float CellSpawnTimeSlice = 2.f;
	
	/** @return Default runtime-generation settings with a freshly generated friendly seed. */
	static FNAssemblyOperationSettings GetDefaultSettings()
	{
		const UNWorldAssemblySettings* AssemblySettings = UNWorldAssemblySettings::Get();
		FNAssemblyOperationSettings Settings;
		
		Settings.Seed = FNSeedGenerator::RandomFriendlySeed();
		Settings.AssemblyDirectionTolerance = AssemblySettings->AssemblyDirectionTolerance;
		Settings.ContextTags.AppendTags(AssemblySettings->AssemblyContextTags);
		Settings.CellSpawnTimeSlice = AssemblySettings->AssemblySpawningCellProxiesTimeSlice;
		Settings.TagCounters = FNGameplayTagCounter(AssemblySettings->AssemblyTagCounters);
		
		return MoveTemp(Settings);
	}

	/** @return Default editor-preview settings (level instances disabled) with a freshly generated seed. */
	static FNAssemblyOperationSettings GetDefaultEditorSettings()
	{
		const UNWorldAssemblySettings* AssemblySettings = UNWorldAssemblySettings::Get();
		FNAssemblyOperationSettings Settings;
		Settings.bCreateLevelInstances = false;
		
		Settings.Seed = FNSeedGenerator::RandomFriendlySeed();
		Settings.AssemblyDirectionTolerance = AssemblySettings->AssemblyDirectionTolerance;
		Settings.ContextTags.AppendTags(AssemblySettings->AssemblyContextTags);
		Settings.CellSpawnTimeSlice = AssemblySettings->AssemblySpawningCellProxiesTimeSlice;
		Settings.TagCounters = FNGameplayTagCounter(AssemblySettings->AssemblyTagCounters);
		
		return MoveTemp(Settings);
	}
};
