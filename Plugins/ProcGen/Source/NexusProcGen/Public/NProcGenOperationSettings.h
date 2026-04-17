// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Math/NSeedGenerator.h"

#include "NProcGenOperationSettings.generated.h"

USTRUCT(BlueprintType) 
struct NEXUSPROCGEN_API FNProcGenOperationSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "General")
	FString Seed;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "General")
	FText DisplayName;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Level Instances")
	bool bReplicateLevelInstances = true;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Level Instances")
	bool bPreLoadLevelInstances = false;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Level Instances")
	bool bLoadLevelInstances = true;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Level Instances")
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
		Settings.bLoadLevelInstances = false;
		Settings.bCreateLevelInstances = false;
		Settings.Seed = FNSeedGenerator::RandomFriendlySeed();
		return MoveTemp(Settings);
	}
};
