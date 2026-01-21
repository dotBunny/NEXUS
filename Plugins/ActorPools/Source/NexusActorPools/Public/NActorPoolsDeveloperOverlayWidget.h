// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "INWidgetMessageProvider.h"
#include "NDeveloperOverlayWidget.h"
#include "NActorPool.h"
#include "NActorPoolsDeveloperOverlayWidget.generated.h"

class UNActorPoolSubsystem;
class UNActorPoolDeveloperObject;
class UNListView;

UCLASS(ClassGroup = "NEXUS", DisplayName = "Actor Pools Developer Overlay Widget", BlueprintType, Abstract)
class NEXUSACTORPOOLS_API UNActorPoolsDeveloperOverlayWidget : public UNDeveloperOverlayWidget, public INWidgetMessageProvider
{
	GENERATED_BODY()
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	void Bind(UWorld* World);
	void Unbind(const UWorld* World);
	
	//~INMessageProvider interface
	virtual bool HasProvidedMessage() const override;
	virtual FString GetProvidedMessage() const override;
	//~End of INMessageProvider interface
	
protected:	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UNListView> ActorPoolList;	
	
	void OnWorldPostInitialization(UWorld* World, FWorldInitializationValues WorldInitializationValues);
	void OnWorldBeginTearDown(UWorld* World);

private:
	void CreateListItem(FNActorPool* ActorPool);
	
	TMap<UWorld*, FDelegateHandle> OnActorPoolAddedDelegates;
	FDelegateHandle AddWorldDelegateHandle;
	FDelegateHandle RemoveWorldDelegateHandle;
	
	float CachedUpdateRate;
	float UpdateTimer;
	
};