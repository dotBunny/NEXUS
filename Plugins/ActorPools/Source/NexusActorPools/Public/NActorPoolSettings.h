// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NFlagsMacros.h"
#include "NActorPoolSettings.generated.h"

UENUM(BlueprintType)
enum ENActorPoolStrategy : uint8
{	
	// Create AActor as needed.
	APS_Create				UMETA(DisplayName = "Create"),
	// Create AActor until MaximumActorCount is reached and stop returning a nullptr in such cases.
	APS_CreateLimited		UMETA(DisplayName = "Create Till Cap"),
	// Create AActor until MaximumActorCount is reached, any requests beyond provide the oldest already spawned AActor in a FIFO behavior.
	APS_CreateRecycleFirst	UMETA(DisplayName = "Create Till Cap, Recycle First"),
	// Create AActor until MaximumActorCount is reached, any requests beyond provide the newest already spawned AActor in a LIFO behavior.
	APS_CreateRecycleLast	UMETA(DisplayName = "Create Till Cap, Recycle Last"),
	// Deploys AActor as needed from fixed pools, exceeding availability results in a nullptr being returned.
	APS_Fixed				UMETA(DisplayName = "Fixed Availability"),
	// Deploys AActor as needed from fixed pools, exceeding availability will return the oldest already spawned AActor in a FIFO behavior.
	APS_FixedRecycleFirst	UMETA(DisplayName = "Fixed Availability, Recycle First"),
	// Deploys AActor as needed from fixed pools, exceeding availability will return the newest already spawned AActor in a LIFO behavior.
	APS_FixedRecycleLast	UMETA(DisplayName = "Fixed Availability, Recycle Last")
};

UENUM(meta=(Bitflags,UseEnumValuesAsMaskValuesInEditor=true))
enum class ENActorPoolFlags : uint8
{
	APF_None = 0								UMETA(DisplayName = "None", Hidden),
	// Should a sweep be done when setting the location of an Actor being spawned?
	APF_SweepBeforeSettingLocation = 1 << 0		UMETA(DisplayName = "Sweep Before Setting Location"),
	// Should the Actor being returned to the pool be moved to a storage location? 
	APF_ReturnToStorageLocation = 1 << 1		UMETA(DisplayName = "Return To Storage Location"),
	// Controls whether Actor construction is deferred when creating new Actors.
	APF_DeferConstruction = 1 << 2				UMETA(DisplayName = "Defer Construction"),
	/**
	 * Should FinishSpawning be called on the Actor when it does not implement INActorPoolItem?
	 * @note This is not used by INActorPoolItems, they have a method which is called for logic to be applied before FinishSpawning is called.
	 */
	APF_ShouldFinishSpawning = 1 << 3			UMETA(DisplayName = "Should Finish Spawning"),
	/**
	 * Safely ensure all actions only actually occur on world authority (server), transparently making the pool networked.
	 */
	APF_ServerOnly = 1 << 4				UMETA(DisplayName = "Server Only"),
	
	APF_DefaultSettings = APF_ReturnToStorageLocation | APF_DeferConstruction | APF_ShouldFinishSpawning | APF_ServerOnly
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
	bool HasFlag_ServerOnly() const
	{
		return N_FLAGS_HAS(Flags, (uint8)ENActorPoolFlags::APF_ServerOnly);
	}
	
	/**
	 * When the pool is being filled during creation, what is the number of prewarmed Actor`s that should be created, either synchronously or divided across a number of frames.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	int32 MinimumActorCount = 10;

	/**
	 * The number of pooled Actors that a pool can use/have. This is tied more to the Strategy being used for what happens when the pool has to create new Actors.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	int32 MaximumActorCount = 100;

	/**
	 * Throttles the number of Actors that can be created per tick. This can be useful to spread the cost of warming a pool up across multiple frames (-1 for unlimited). 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	int32 CreateObjectsPerTick = -1;

	/**
	 * Determines the approach taken when the pool does not have any Actors remaining in the "in" pool and needs to create one (or reuse)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	TEnumAsByte<ENActorPoolStrategy> Strategy = ENActorPoolStrategy::APS_Create;
	
	/**
	 * The behavioral flags to evaluate when doing operations with this pool.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling", meta=(Bitmask,BitmaskEnum="/Script/NexusActorPools.ENActorPoolFlags"))
	uint8 Flags = static_cast<uint8>(ENActorPoolFlags::APF_DefaultSettings);

	/**
	 * The location to move an Actor when it is returned to the pool for later reuse (if enabled). This also gets applied to newly created Actors as well.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	FVector StorageLocation = FVector::ZeroVector;

	/**
	 * The default applied transform when creating an actor, including the base assumption that the AActor's root component is not scaled.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	FTransform DefaultTransform = FTransform(FRotator::ZeroRotator, FVector::Zero(), FVector::One());
};