// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NActorPool.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/StreamableManager.h"
#include "NSpawnActorBlueprintAsyncAction.generated.h"

class UNActorPoolSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpawnActorAsyncOutputPin, AActor*, SpawnedActor);

/**
 * Async Blueprint action that soft-loads an AActor class and then spawns an instance of it from the Actor Pool subsystem.
 */
UCLASS()
class NEXUSACTORPOOLS_API UNSpawnActorBlueprintAsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

#if WITH_TESTS
	friend class UNSpawnActorBlueprintAsyncActionTests_OnHasPool_NullPool;
	friend class UNSpawnActorBlueprintAsyncActionTests_OnHasPool_MismatchedTemplate;
	friend class UNSpawnActorBlueprintAsyncActionTests_HandleCleanup_OnDestroy;
	friend class UNSpawnActorBlueprintAsyncActionTests_OnLoaded_ExpiredContext;
#endif // WITH_TESTS

public:
	/**
	 * Spawns an actor from a given pool asynchronously, creating a pool as necessary.
	 * @note Primarily used to decouple hard references to Actors.
	 * @param WorldContextObject Context used to resolve the Actor Pool subsystem.
	 * @param ActorClass The soft class to load and then spawn from the pool.
	 * @param Position World-space position to spawn the actor at.
	 * @param Rotation World-space rotation to apply to the spawned actor.
	 * @return The async action instance that BP will attach its output pins to.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Spawn Actor Async", Category = "NEXUS|Actor Pools",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UNSpawnActorBlueprintAsyncAction* SpawnActorAsync(UObject* WorldContextObject, TSoftClassPtr<AActor> ActorClass, FVector Position, FRotator Rotation);

	/** Fires once the class has been loaded and the pooled actor has been spawned. SpawnedActor is null on failure. */
	UPROPERTY(BlueprintAssignable)
	FSpawnActorAsyncOutputPin Completed;

	virtual void Activate() override;
	virtual void BeginDestroy() override;

private:
	FDelegateHandle OnCreatedPoolHandle;
	TWeakObjectPtr<UObject> WorldContext;
	TSoftClassPtr<AActor> ActorClass;
	FVector Position = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;
	TSharedPtr<FStreamableHandle> StreamingHandle;

	/**
	 * Resolves the Actor Pool subsystem from the (weak) world context, tolerating an expired context.
	 * @return The subsystem, or null if the context/world has gone away (e.g. a level transition completed
	 *         while the class was still streaming in).
	 * @note Async actions outlive their world context via the game instance, so this must never assume the
	 *       context is still valid — unlike N_GET_WORLD_FROM_CONTEXT, whose shipping arm dereferences it raw.
	 */
	UNActorPoolSubsystem* ResolveActorPoolSubsystem() const;

	void OnLoaded();
	void OnHasPool(FNActorPool* ActorPool);
};
