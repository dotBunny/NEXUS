// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Widgets/NDeveloperOverlay.h"
#include "NActorPool.h"
#include "NActorPoolsDeveloperOverlay.generated.h"

class UNActorPoolSubsystem;
class UNActorPoolDeveloperObject;
class UNListView;

/**
 * Developer overlay widget that lists every FNActorPool in the active world(s) with live in/out counts.
 * @see <a href="https://nexus-framework.com/docs/plugins/actor-pools/developer-overlay/">UNActorPoolsDeveloperOverlay</a>
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Actor Pools Developer Overlay", BlueprintType, Abstract)
class NEXUSACTORPOOLS_API UNActorPoolsDeveloperOverlay : public UNDeveloperOverlay
{
	GENERATED_BODY()

public:

	/** Subscribe to the actor-pool subsystem hosted by World so its pool-added events drive this overlay. */
	virtual void BindWorld(UWorld* World) override;
	/** Drop the subscription to the actor-pool subsystem hosted by World. */
	virtual void UnbindWorld(const UWorld* World) override;

	/** @return The list view widget displaying the pools. */
	TObjectPtr<UNListView> GetActorPoolList() const { return ActorPoolList; }

protected:

	//~UUserWidget
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//End UUserWidget

	/** The bound list view used to display pool rows. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> ActorPoolList;

private:

	/** Add a row to ActorPoolList representing the supplied pool. */
	void CreateListItem(FNActorPool* ActorPool);
	/** Refresh the overlay banner's text based on whether any pools are currently live. */
	void UpdateBanner() const;

	/** Per-world delegate handles used to listen for new pools being added. */
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TMap<UWorld*, FDelegateHandle> OnActorPoolAddedDelegates;

	/** Placeholder text shown when no pools are present. */
	FText NoActorPoolsFoundText = NSLOCTEXT("NexusActorPools", "NoActorPoolsFound", "No Actor Pools Found");

	/** Accumulator used to throttle list refresh cadence. */
	float UpdateTimer;

};