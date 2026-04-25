// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "NActorPool.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/StreamableManager.h"
#include "NSpawnActorBlueprintAsyncAction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpawnActorAsyncOutputPin, AActor*, SpawnedActor);

/**
 * Async Blueprint action that soft-loads an AActor class and then spawns an instance of it from the Actor Pool subsystem.
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/types/spawn-actor-blueprint-async-action/">UNSpawnActorBlueprintAsyncAction</a>
 */
UCLASS()
class NEXUSACTORPOOLS_API UNSpawnActorBlueprintAsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

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

private:
	TWeakObjectPtr<UObject> WorldContext;
	TSoftClassPtr<AActor> ActorClass;
	FVector Position = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;
	TSharedPtr<FStreamableHandle> StreamingHandle;
	FNActorPool* ActorPool;
	
	void OnLoaded();
	void OnHasPool();
};
