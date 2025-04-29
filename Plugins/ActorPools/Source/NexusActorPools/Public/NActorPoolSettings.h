// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NActorPoolSettings.generated.h"

UENUM(BlueprintType)
enum ENActorPoolStrategy : uint8
{
	APS_Create				UMETA(DisplayName = "Create"),
	APS_CreateLimited		UMETA(DisplayName = "Create Till Cap"),
	APS_CreateRecycleFirst	UMETA(DisplayName = "Create Till Cap, Recycle First"),
	APS_CreateRecycleLast	UMETA(DisplayName = "Create Till Cap, Recycle Last"),
	APS_Fixed				UMETA(DisplayName = "Fixed Availabilty"),
	APS_FixedRecycleFirst	UMETA(DisplayName = "Fixed Availabilty, Recycle First"),
	APS_FixedRecycleLast	UMETA(DisplayName = "Fixed Availabilty, Recycle Last")
};

USTRUCT(BlueprintType)
struct NEXUSACTORPOOLS_API FNActorPoolSettings
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	int MinimumActorCount = 10; // TODO: Set this to 10? when we have figured out the npc issue
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	int MaximumActorCount = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	int CreateObjectsPerTick = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	bool bAllowCreateMoreObjects = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	TEnumAsByte<ENActorPoolStrategy> Strategy = ENActorPoolStrategy::APS_Create;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	bool bSpawnSweepBeforeSettingLocation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	bool bReturnMoveToLocation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	FVector ReturnMoveLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	bool bDeferConstruction = true;

	/**
	 * Should FinishSpawning be called on the Actor when it does not implement INActorPoolItem.
	 * @remark This is not used by INActorPoolItems, they have method which is called for logic to be applied before FinishSpawning is called.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	bool bShouldFinishSpawning = true;
};