// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Math/NSeedGenerator.h"

#include "NProcGenOperationSettings.generated.h"

USTRUCT(BlueprintType) 
struct NEXUSPROCGEN_API FNProcGenOperationSettings
{
	GENERATED_BODY()
	
	UPROPERTY()
	FString Seed;
	
	UPROPERTY()
	FText DisplayName;
	
	UPROPERTY()
	bool bReplicateLevelInstances = true;
	
	UPROPERTY()
	bool bLoadLevelInstances = true;
	
	UPROPERTY()
	bool bCreateLevelInstances = true;

	static FNProcGenOperationSettings GetDefaultSettings()
	{
		FNProcGenOperationSettings Settings;
		Settings.Seed = FNSeedGenerator::RandomFriendlySeed();
		return MoveTemp(Settings);
	}
	
	static FNProcGenOperationSettings GetDefaultEditorSettings()
	{
		FNProcGenOperationSettings Settings;
		Settings.Seed = FNSeedGenerator::RandomFriendlySeed();
		return MoveTemp(Settings);
	}
};
