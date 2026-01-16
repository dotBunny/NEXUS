// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolsDeveloperOverlayWidget.h"
#include "Components/NListView.h"

void UNActorPoolsDeveloperOverlayWidget::NativeConstruct()
{
	System = UNActorPoolSubsystem::Get(GetWorld());
	
	// Build out known list
	TArray<FNActorPool*> KnownPools = System->GetAllPools();
	for (const auto Pool : KnownPools)
	{
		CreateListItem(Pool);
	}
	
	// Add delegate for future pools
	OnActorPoolAddedDelegateHandle = System->OnActorPoolAdded.AddLambda([this] (FNActorPool* ActorPool)
	{
		this->CreateListItem(ActorPool);
	});
	
	Super::NativeConstruct();
}

void UNActorPoolsDeveloperOverlayWidget::NativeDestruct()
{
	System->OnActorPoolAdded.Remove(OnActorPoolAddedDelegateHandle);
	
	TArray<UObject*> Items = ActorPoolList->GetListItems();
	const int ItemCount = Items.Num();
	for (int i = ItemCount - 1; i >= 0; i--)
	{
		UNActorPoolDeveloperObject* Object = Cast<UNActorPoolDeveloperObject>(Items[i]);
		Object->RemoveFromRoot();
	}
	ActorPoolList->ClearListItems();

	
	Super::NativeDestruct();
}

void UNActorPoolsDeveloperOverlayWidget::CreateListItem(FNActorPool* ActorPool)
{
	UNActorPoolDeveloperObject* Object = NewObject<UNActorPoolDeveloperObject>(this, UNActorPoolDeveloperObject::StaticClass());
	Object->SetPool(ActorPool);
	ActorPoolList->AddItem(Object);
}
