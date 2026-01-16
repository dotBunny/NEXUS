// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "NActorPoolSubsystem.h"
#include "NDeveloperOverlayWidget.h"
#include "NActorPool.h"
#include "NActorPoolsDeveloperOverlayWidget.generated.h"

class UNActorPoolDeveloperObject;
class UNListView;

UCLASS(ClassGroup = "NEXUS", DisplayName = "Actor Pools Developer Overlay Widget", BlueprintType, Abstract)
class NEXUSACTORPOOLS_API UNActorPoolsDeveloperOverlayWidget : public UNDeveloperOverlayWidget
{
	GENERATED_BODY()
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	
protected:	
	UPROPERTY(BlueprintReadOnly)
	TArray<UNActorPoolDeveloperObject*> ActorPoolObjects;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> ActorPoolList;	
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNActorPoolSubsystem> System;

private:
	void CreateListItem(FNActorPool* ActorPool);
	FDelegateHandle OnActorPoolAddedDelegateHandle;
};


UCLASS(BlueprintType)
class NEXUSACTORPOOLS_API UNActorPoolDeveloperObject : public UObject
{
	GENERATED_BODY()

public:
	void SetPool(FNActorPool* NewPool)
	{
		this->Pool = NewPool;
	}
	
	UFUNCTION(BlueprintCallable)
	int GetInCount() const
	{
		return Pool->GetInCount();
	}
	
	UFUNCTION(BlueprintCallable)
	int GetOutCount() const
	{
		return Pool->GetOutCount();
	}
private:	
	FNActorPool* Pool;
};