// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "Widgets/NDeveloperOverlay.h"
#include "NActorPool.h"
#include "NActorPoolsDeveloperOverlay.generated.h"

class UNActorPoolSubsystem;
class UNActorPoolDeveloperObject;
class UNListView;

UCLASS(ClassGroup = "NEXUS", DisplayName = "Actor Pools Developer Overlay", BlueprintType, Abstract)
class NEXUSACTORPOOLS_API UNActorPoolsDeveloperOverlay : public UNDeveloperOverlay
{
	GENERATED_BODY()
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	void Bind(UWorld* World);
	void Unbind(const UWorld* World, bool bClearItems = true);

protected:	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> ActorPoolList;	
	
	void OnWorldPostInitialization(UWorld* World, FWorldInitializationValues WorldInitializationValues);
	void OnWorldBeginTearDown(UWorld* World);

private:
	
	void CreateListItem(FNActorPool* ActorPool);
	void UpdateBanner() const;
	
	TMap<UWorld*, FDelegateHandle> OnActorPoolAddedDelegates;
	FDelegateHandle AddWorldDelegateHandle;
	FDelegateHandle RemoveWorldDelegateHandle;
	
	float CachedUpdateRate;
	float UpdateTimer;
	
};