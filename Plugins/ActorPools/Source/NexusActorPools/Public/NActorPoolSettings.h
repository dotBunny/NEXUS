// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NFlagsMacros.h"
#include "NActorPoolSettings.generated.h"

UENUM(BlueprintType)
enum class ENActorPoolStrategy : uint8
{
	// Create AActor as needed.
	Create = 0				UMETA(DisplayName = "Create"),
	// Create AActor until MaximumActorCount is reached and stop returning a nullptr in such cases.
	CreateLimited = 1		UMETA(DisplayName = "Create Till Cap"),
	// Create AActor until MaximumActorCount is reached, any requests beyond provide the oldest already spawned AActor in a FIFO behavior.
	CreateRecycleFirst = 2	UMETA(DisplayName = "Create Till Cap, Recycle First"),
	// Create AActor until MaximumActorCount is reached, any requests beyond provide the newest already spawned AActor in a LIFO behavior.
	CreateRecycleLast = 3	UMETA(DisplayName = "Create Till Cap, Recycle Last"),
	// Deploys AActor as needed from fixed pools, exceeding availability results in a nullptr being returned.
	Fixed = 4				UMETA(DisplayName = "Fixed Availability"),
	// Deploys AActor as needed from fixed pools, exceeding availability will return the oldest already spawned AActor in a FIFO behavior.
	FixedRecycleFirst = 5	UMETA(DisplayName = "Fixed Availability, Recycle First"),
	// Deploys AActor as needed from fixed pools, exceeding availability will return the newest already spawned AActor in a LIFO behavior.
	FixedRecycleLast = 6	UMETA(DisplayName = "Fixed Availability, Recycle Last")
};

UENUM(meta=(Bitflags,UseEnumValuesAsMaskValuesInEditor=true))
enum class ENActorPoolFlags : uint8
{
	None = 0								UMETA(DisplayName = "None", Hidden),
	// Should a sweep be done when setting the location of an Actor being spawned?
	SweepBeforeSettingLocation = 1 << 0		UMETA(DisplayName = "Sweep Before Setting Location"),
	// Should the Actor being returned to the pool be moved to a storage location? 
	ReturnToStorageLocation = 1 << 1		UMETA(DisplayName = "Return To Storage Location"),
	// Controls whether Actor construction is deferred when creating new Actors.
	DeferConstruction = 1 << 2				UMETA(DisplayName = "Defer Construction"),
	/**
	 * Should FinishSpawning be called on the Actor when it does not implement INActorPoolItem?
	 * @note This is not used by INActorPoolItems, they have a method which is called for logic to be applied before FinishSpawning is called.
	 */
	ShouldFinishSpawning = 1 << 3			UMETA(DisplayName = "Should Finish Spawning"),
	/**
	 * Safely ensure all actions only actually occur on world authority (server), transparently making the pool networked.
	 */
	ServerOnly = 1 << 4						UMETA(DisplayName = "Server Only"),
	/**
	 * Broadcast destroy event on the Actor through the operational change state delegate.
	 */
	BroadcastDestroy = 1 << 5				UMETA(DisplayName = "Broadcast Destroy"),
	/**
	 * Should an Actor's network dormancy be updated based on state?
	 */
	SetNetDormancy = 1 << 6					UMETA(DisplayName = "Set Net Dormancy"),
	
	DefaultSettings = ReturnToStorageLocation | DeferConstruction | ShouldFinishSpawning | ServerOnly | SetNetDormancy
};
ENUM_CLASS_FLAGS(ENActorPoolFlags)

/**
 * A Blueprint-compatible struct that defines configuration parameters for managing object pooling inside a FNActorPool.
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-settings/">FNActorPoolSettings</a>
 */
USTRUCT(BlueprintType)
struct NEXUSACTORPOOLS_API FNActorPoolSettings
{
	GENERATED_BODY()

public:

	FORCEINLINE bool HasFlag_DeferConstruction() const
	{
		return N_FLAGS_HAS(Flags, (uint8)ENActorPoolFlags::DeferConstruction);
	}
	FORCEINLINE bool HasFlag_ShouldFinishSpawning() const
	{
		return N_FLAGS_HAS(Flags, (uint8)ENActorPoolFlags::ShouldFinishSpawning);
	}
	FORCEINLINE bool HasFlag_ReturnToStorageLocation() const
	{
		return N_FLAGS_HAS(Flags, (uint8)ENActorPoolFlags::ReturnToStorageLocation);
	}
	FORCEINLINE bool HasFlag_SweepBeforeSettingLocation() const
	{
		return N_FLAGS_HAS(Flags, (uint8)ENActorPoolFlags::SweepBeforeSettingLocation);
	}
	FORCEINLINE bool HasFlag_ServerOnly() const
	{
		return N_FLAGS_HAS(Flags, (uint8)ENActorPoolFlags::ServerOnly);
	}
	FORCEINLINE bool HasFlag_BroadcastDestroy() const
	{
		return N_FLAGS_HAS(Flags, (uint8)ENActorPoolFlags::BroadcastDestroy);
	}
	FORCEINLINE bool HasFlag_SetNetDormancy() const
	{
		return N_FLAGS_HAS(Flags, (uint8)ENActorPoolFlags::SetNetDormancy);
	}
	
	/**
	 * When the pool is being filled during creation, what is the number of prewarmed Actor`s that should be created, either synchronously or divided across a number of frames.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling", meta = (ClampMin="0", UIMin="0", SliderExponent = 1))
	int32 MinimumActorCount = 10;

	/**
	 * The number of pooled Actors that a pool can use/have. This is tied more to the Strategy being used for what happens when the pool has to create new Actors.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling", meta = (ClampMin="0", UIMin="0", SliderExponent = 1))
	int32 MaximumActorCount = 100;

	/**
	 * Throttles the number of Actors that can be created per tick. This can be useful to spread the cost of warming a pool up across multiple frames (-1 for unlimited). 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling", meta = (ClampMin="-1", ClampMax="500", UIMin="-1", UIMax="500", SliderExponent = 1))
	int32 CreateObjectsPerTick = -1;

	/**
	 * Determines the approach taken when the pool does not have any Actors remaining in the "in" pool and needs to create one (or reuse)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	ENActorPoolStrategy Strategy = ENActorPoolStrategy::Create;
	
	/**
	 * The behavioral flags to evaluate when doing operations with this pool.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling", meta=(Bitmask,BitmaskEnum="/Script/NexusActorPools.ENActorPoolFlags"))
	uint8 Flags = static_cast<uint8>(ENActorPoolFlags::DefaultSettings);
	
	/**
	 * The default applied transform when creating an actor, with the location component being used as the storage location when an actor is returned to the pool, and the scale applied when spawned.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	FTransform DefaultTransform = FTransform(FRotator::ZeroRotator, FVector::Zero(), FVector::One());
};