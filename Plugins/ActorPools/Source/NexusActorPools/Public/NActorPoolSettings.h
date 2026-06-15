// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Macros/NFlagsMacros.h"
#include "NActorPoolSettings.generated.h"

/**
 * Strategy used by an FNActorPool when a new Actor is requested but no free Actor is available.
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-settings/#creation-strategies">ENActorPoolStrategy</a>
 */
UENUM(BlueprintType)
enum class ENActorPoolStrategy : uint8
{
	/** Create AActor as needed. */
	Create = 0,
	/** Create AActor until MaximumActorCount is reached and stop, returning a nullptr in such cases. */
	CreateLimited = 1		UMETA(DisplayName = "Create Till Cap"),
	/** Create AActor until MaximumActorCount is reached, any requests beyond provide the oldest already spawned AActor in a FIFO behavior. */
	CreateRecycleFirst = 2	UMETA(DisplayName = "Create Till Cap, Recycle First"),
	/** Create AActor until MaximumActorCount is reached, any requests beyond provide the newest already spawned AActor in a LIFO behavior. */
	CreateRecycleLast = 3	UMETA(DisplayName = "Create Till Cap, Recycle Last"),
	/** Deploys AActor as needed from fixed pools, exceeding availability results in a nullptr being returned. */
	Fixed = 4				UMETA(DisplayName = "Fixed Availability"),
	/** Deploys AActor as needed from fixed pools, exceeding availability will return the oldest already spawned AActor in a FIFO behavior. */
	FixedRecycleFirst = 5	UMETA(DisplayName = "Fixed Availability, Recycle First"),
	/** Deploys AActor as needed from fixed pools, exceeding availability will return the newest already spawned AActor in a LIFO behavior. */
	FixedRecycleLast = 6	UMETA(DisplayName = "Fixed Availability, Recycle Last")
};

/**
 * Bitflag options that modify the behavior of an FNActorPool and the Actors it manages.
 */
UENUM(meta=(Bitflags,UseEnumValuesAsMaskValuesInEditor=true))
enum class ENActorPoolFlags : uint8
{
	None = 0 UMETA(Hidden),
	/** Should a sweep be done when setting the location of an Actor being spawned? */
	SweepBeforeSettingLocation = 1,
	/** Should the Actor being returned to the pool be moved to a storage transform? */
	ReturnToStorage = 2,
	/** Controls whether Actor construction is deferred when creating new Actors. */
	DeferConstruction = 4,
	/** Safely ensure all actions only actually occur on world authority (server), transparently making the pool networked. */
	ServerOnly = 16,
	/** Broadcast the released-from-pool event on the Actor through the operational change state delegate. */
	BroadcastRelease = 32,
	/** Should an Actor's network dormancy be updated based on state? */
	SetNetDormancy = 64,
	/**
	 * Should the UFunctions (void)OnCreatedByActorPool, (void)OnSpawnedFromActorPool, (void)OnReturnToActorPool, and (void)OnReleasedFromActorPool be invoked to simulate an interface callback to AActor-based blueprints?
	 * @note This is only applicable to non-interfaced AActors, as interfaced Actors have their own interface callback mechanism.
	 */
	InvokeUFunctions = 128 UMETA(DisplayName = "Invoke UFunctions"),
};
ENUM_CLASS_FLAGS(ENActorPoolFlags)


/**
 * Bitflags declaring which optional capabilities an FNActorPool advertises, queried before invoking the
 * corresponding operation (e.g. ReturnAll) so callers can branch on what a given pool supports.
 */
UENUM(meta=(Bitflags,UseEnumValuesAsMaskValuesInEditor=true))
enum class ENActorPoolSupportFlags : uint8
{
	None = 0 UMETA(Hidden),
	/** Does this pool support the use of the ReturnAll method? */
	ReturnAll = 1,
};
ENUM_CLASS_FLAGS(ENActorPoolSupportFlags)

namespace NEXUS::ActorPools
{
	constexpr uint8 DefaultFlags = static_cast<uint8>(ENActorPoolFlags::ReturnToStorage) | 
								  static_cast<uint8>(ENActorPoolFlags::DeferConstruction) | 
								  static_cast<uint8>(ENActorPoolFlags::ServerOnly) | 
								  static_cast<uint8>(ENActorPoolFlags::SetNetDormancy);
	
	constexpr uint8 DefaultSupportFlags = 0;
}

/**
 * A Blueprint-compatible struct that defines configuration parameters for managing object pooling inside a FNActorPool.
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/types/actor-pool-settings/">FNActorPoolSettings</a>
 */
USTRUCT(BlueprintType)
struct NEXUSACTORPOOLS_API FNActorPoolSettings
{
	GENERATED_BODY()

public:
	
	/** @return true if the ReturnAll support flag is set. */
	FORCEINLINE bool HasSupportFlag_ReturnAll() const
	{
		return N_FLAGS_HAS_UINT8(SupportFlags, ENActorPoolSupportFlags::ReturnAll);
	}

	/** @return true if the DeferConstruction flag is set. */
	FORCEINLINE bool HasFlag_DeferConstruction() const
	{
		return N_FLAGS_HAS_UINT8(Flags, ENActorPoolFlags::DeferConstruction);
	}
	/** @return true if the ReturnToStorage flag is set. */
	FORCEINLINE bool HasFlag_ReturnToStorage() const
	{
		return N_FLAGS_HAS_UINT8(Flags, ENActorPoolFlags::ReturnToStorage);
	}
	/** @return true if the SweepBeforeSettingLocation flag is set. */
	FORCEINLINE bool HasFlag_SweepBeforeSettingLocation() const
	{
		return N_FLAGS_HAS_UINT8(Flags, ENActorPoolFlags::SweepBeforeSettingLocation);
	}
	/** @return true if the ServerOnly flag is set. */
	FORCEINLINE bool HasFlag_ServerOnly() const
	{
		return N_FLAGS_HAS_UINT8(Flags, ENActorPoolFlags::ServerOnly);
	}
	/** @return true if the BroadcastRelease flag is set. */
	FORCEINLINE bool HasFlag_BroadcastRelease() const
	{
		return N_FLAGS_HAS_UINT8(Flags, ENActorPoolFlags::BroadcastRelease);
	}
	/** @return true if the SetNetDormancy flag is set. */
	FORCEINLINE bool HasFlag_SetNetDormancy() const
	{
		return N_FLAGS_HAS_UINT8(Flags, ENActorPoolFlags::SetNetDormancy);
	}

	/** @return true if the InvokeUFunctions flag is set. */
	FORCEINLINE bool HasFlag_InvokeUFunctions() const
	{
		return N_FLAGS_HAS_UINT8(Flags, ENActorPoolFlags::InvokeUFunctions);
	}
	
	/** When the pool is being filled during creation, what is the number of prewarmed Actor`s that should be created, either synchronously or divided across a number of frames. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling", meta = (ClampMin="0", UIMin="0", SliderExponent = 1))
	int32 MinimumActorCount = 10;

	/** The number of pooled Actors that a pool can use/have. This is tied more to the Strategy being used for what happens when the pool has to create new Actors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling", meta = (ClampMin="1", UIMin="1", SliderExponent = 1))
	int32 MaximumActorCount = 100;

	/** Throttles the number of Actors that can be created per tick. This can be useful to spread the cost of warming a pool up across multiple frames (-1 for unlimited). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling", meta = (ClampMin="-1", ClampMax="500", UIMin="-1", UIMax="500", SliderExponent = 1))
	int32 CreateObjectsPerTick = -1;

	/** Determines the approach taken when the pool does not have any Actors remaining in the "in" pool and needs to create one (or reuse) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	ENActorPoolStrategy Strategy = ENActorPoolStrategy::Create;
	
	/** The behavioral flags to evaluate when doing operations with this pool. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling", meta=(Bitmask,BitmaskEnum="/Script/NexusActorPools.ENActorPoolFlags"))
	uint8 Flags = NEXUS::ActorPools::DefaultFlags;
	
	/** Flags outling what features this pool supports. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling", meta=(Bitmask,BitmaskEnum="/Script/NexusActorPools.ENActorPoolSupportFlags"))
	uint8 SupportFlags = NEXUS::ActorPools::DefaultSupportFlags;
	
	/** The default applied transform when creating an actor, as well as where and how an actor is stored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	FTransform StorageTransform = FTransform(FRotator::ZeroRotator, FVector::Zero(), FVector::One());
	
	/** The base transform applied when spawning an actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Pooling")
	FTransform SpawnedTransform = FTransform(FRotator::ZeroRotator, FVector::Zero(), FVector::One());
};