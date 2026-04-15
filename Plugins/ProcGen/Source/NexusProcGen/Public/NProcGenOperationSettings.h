// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Math/NSeedGenerator.h"

#include "NProcGenOperationSettings.generated.h"

USTRUCT(BlueprintType) 
struct NEXUSPROCGEN_API FNProcGenOperationSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	FString Seed;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	FText DisplayName;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	bool bReplicateLevelInstances = true;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	bool bLoadLevelInstances = true;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
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
