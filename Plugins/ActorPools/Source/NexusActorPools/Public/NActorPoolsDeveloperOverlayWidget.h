// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NDeveloperOverlayWidget.h"
#include "NActorPool.h"
#include "NActorPoolsDeveloperOverlayWidget.generated.h"

class UNActorPoolSubsystem;
class UNActorPoolDeveloperObject;
class UNListView;

UCLASS(ClassGroup = "NEXUS", DisplayName = "Actor Pools Developer Overlay Widget", BlueprintType, Abstract)
class NEXUSACTORPOOLS_API UNActorPoolsDeveloperOverlayWidget : public UNDeveloperOverlayWidget
{
	GENERATED_BODY()

	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void Bind(UWorld* World);
	void Unbind(UWorld* World);
	
protected:	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> ActorPoolList;	
	
	void OnWorldPostInitialization(UWorld* World, FWorldInitializationValues WorldInitializationValues);
	void OnWorldBeginTearDown(UWorld* World);

private:
	void CreateListItem(FNActorPool* ActorPool);
	
	
	
	TMap<UWorld*, FDelegateHandle> WorldToHandleMap;
	FDelegateHandle AddWorldDelegateHandle;
	FDelegateHandle RemoveWorldDelegateHandle;
};