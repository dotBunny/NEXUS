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

	void Bind(UWorld* World);
	void Unbind(const UWorld* World);

public:
	/** @return The list view widget displaying the pools. */
	TObjectPtr<UNListView> GetActorPoolList() const { return ActorPoolList; }

protected:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void OnWorldPostInitialization(UWorld* World, FWorldInitializationValues WorldInitializationValues);
	void OnWorldBeginTearDown(UWorld* World);

	/** The bound list view used to display pool rows. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> ActorPoolList;

private:

	void CreateListItem(FNActorPool* ActorPool);
	void UpdateBanner() const;

	/** Per-world delegate handles used to listen for new pools being added. */
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TMap<UWorld*, FDelegateHandle> OnActorPoolAddedDelegates;

	/** Delegate handle for FWorldDelegates::OnPostWorldInitialization. */
	FDelegateHandle AddWorldDelegateHandle;

	/** Delegate handle for FWorldDelegates::OnWorldBeginTearDown. */
	FDelegateHandle RemoveWorldDelegateHandle;

	/** Placeholder text shown when no pools are present. */
	FText NoActorPoolsFoundText = NSLOCTEXT("NexusActorPools", "NoActorPoolsFound", "No Actor Pools Found");

	/** Accumulator used to throttle list refresh cadence. */
	float UpdateTimer;

};