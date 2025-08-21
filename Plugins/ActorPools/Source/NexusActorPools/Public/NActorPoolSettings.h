// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "Macros/NFlagsMacros.h"

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

UENUM(meta=(Bitflags,UseEnumValuesAsMaskValuesInEditor=true))
enum class ENActorPoolFlags : uint8
{
	APF_None = 0								UMETA(DisplayName = "None", Hidden),
	/**
	* Should a sweep be done when setting the location of an Actor being spawned.
	*/
	APF_SweepBeforeSettingLocation = 1 << 0		UMETA(DisplayName = "Sweep Before Setting Location"),
	/**
	* Should the Actor being returned to the pool be moved to a storage location? 
	*/
	APF_ReturnToStorageLocation = 1 << 1		UMETA(DisplayName = "Return To Storage Location"),
	/**
	 * Controls whether Actor construction is deferred when creating new Actors.
	 */
	APF_DeferConstruction = 1 << 2				UMETA(DisplayName = "Defer Construction"),
	/**
	 * Should FinishSpawning be called on the Actor when it does not implement INActorPoolItem.
	 * @remark This is not used by INActorPoolItems, they have method which is called for logic to be applied before FinishSpawning is called.
	 */
	APF_ShouldFinishSpawning = 1 << 3			UMETA(DisplayName = "Should Finish Spawning"),
	
	APF_DefaultSettings = APF_ReturnToStorageLocation | APF_DeferConstruction | APF_ShouldFinishSpawning
};
ENUM_CLASS_FLAGS(ENActorPoolFlags)

USTRUCT(BlueprintType)
struct NEXUSACTORPOOLS_API FNActorPoolSettings
{
	GENERATED_BODY()

public:

	bool HasFlag_DeferConstruction() const
	{
		return N_FLAGS_HAS(Flags, (uint8)ENActorPoolFlags::APF_DeferConstruction);
	}
	bool HasFlag_ShouldFinishSpawning() const
	{
		return N_FLAGS_HAS(Flags, (uint8)ENActorPoolFlags::APF_ShouldFinishSpawning);
	}
	bool HasFlag_ReturnToStorageLocation() const
	{
		return N_FLAGS_HAS(Flags, (uint8)ENActorPoolFlags::APF_ReturnToStorageLocation);
	}
	bool HasFlag_SweepBeforeSettingLocation() const
	{
		return N_FLAGS_HAS(Flags, (uint8)ENActorPoolFlags::APF_SweepBeforeSettingLocation);
	}
	
	/**
	 * When the pool is being filled during creation, what is the number of prewarmed Actor`s that should be created, either synchronously or divided across a number of frames.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	int MinimumActorCount = 10;

	/**
	 * The number of pooled Actors that a pool can use/have. This is tied more to the Strategy being used for what happens when the pool has to create new Actors.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	int MaximumActorCount = 100;

	/**
	 * Throttles the number of Actors that can be created per tick. This can be useful to spread the cost of warming a pool up across multiple frames (-1 for unlimited). 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	int CreateObjectsPerTick = -1;

	/**
	 * Determines the approach taken when the pool does not have any Actors remaining in the "in" pool, and needs to create one (or reuse)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	TEnumAsByte<ENActorPoolStrategy> Strategy = ENActorPoolStrategy::APS_Create;
	
	/**
	 * The behavioral flags to evaluate when doing operations with this pool.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Bitmask,BitmaskEnum="/Script/NexusActorPools.ENActorPoolFlags"))
	uint8 Flags = static_cast<uint8>(ENActorPoolFlags::APF_DefaultSettings);

	/**
	 * The location to move an Actor when it is returned to the pool for later reuse (if enabled). This also gets applied to newly created Actors as well.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling", meta = (EditCondition ="(Flags & ENActorPoolFlags::APF_ReturnToStorageLocation) == ENActorPoolFlags::APF_ReturnToStorageLocation"))
	FVector StorageLocation = FVector::ZeroVector;
};