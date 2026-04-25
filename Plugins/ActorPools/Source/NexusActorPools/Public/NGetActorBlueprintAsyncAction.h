// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/StreamableManager.h"
#include "NGetActorBlueprintAsyncAction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGetActorAsyncOutputPin, AActor*, SpawnedActor);

/**
 * Async Blueprint action that soft-loads an AActor class and then gets an instance of it from the Actor Pool subsystem.
 */
UCLASS()
class NEXUSACTORPOOLS_API UNGetActorBlueprintAsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Gets an actor from a given pool asynchronously, creating a pool as necessary.
	 * @note Primarily used to decouple hard references to Actors.
	 * @note This does not trigger any events on the given actor, it does not activate them in any way.
	 * @param WorldContextObject Context used to resolve the Actor Pool subsystem.
	 * @param ActorClass The soft class to load and then spawn from the pool.
	 * @return The async action instance that BP will attach its output pins to.
	 */
	UFUNCTION(BlueprintCallable, DisplayName="Get Actor Async", Category = "NEXUS|Actor Pools",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UNGetActorBlueprintAsyncAction* GetActorAsync(UObject* WorldContextObject, TSoftClassPtr<AActor> ActorClass);

	/** Fires once the class has been loaded and the pooled actor has been spawned. SpawnedActor is null on failure. */
	UPROPERTY(BlueprintAssignable)
	FGetActorAsyncOutputPin Completed;

	virtual void Activate() override;

private:
	TWeakObjectPtr<UObject> WorldContext;
	TSoftClassPtr<AActor> ActorClass;
	TSharedPtr<FStreamableHandle> StreamingHandle;

	void OnLoaded();
	void OnHasPool();
};
