// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Widgets/NDeveloperOverlay.h"
#include "NActorPool.h"
#include "NActorPoolsDeveloperOverlay.generated.h"

class UNActorPoolSubsystem;
class UNActorPoolDeveloperObject;
class UNListView;

UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Actor Pools Developer Overlay", BlueprintType, Abstract)
class NEXUSACTORPOOLS_API UNActorPoolsDeveloperOverlay : public UNDeveloperOverlay
{
	GENERATED_BODY()
	
	void Bind(UWorld* World);
	void Unbind(const UWorld* World);
	
public:
	TObjectPtr<UNListView> GetActorPoolList() const { return ActorPoolList; }

protected:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	void OnWorldPostInitialization(UWorld* World, FWorldInitializationValues WorldInitializationValues);
	void OnWorldBeginTearDown(UWorld* World);
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> ActorPoolList;

private:
	
	void CreateListItem(FNActorPool* ActorPool);
	void UpdateBanner() const;
	
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TMap<UWorld*, FDelegateHandle> OnActorPoolAddedDelegates;
	
	FDelegateHandle AddWorldDelegateHandle;
	FDelegateHandle RemoveWorldDelegateHandle;
	FText NoActorPoolsFoundText = NSLOCTEXT("NexusActorPools", "NoActorPoolsFound", "No Actor Pools Found");
	float UpdateTimer;
	
};