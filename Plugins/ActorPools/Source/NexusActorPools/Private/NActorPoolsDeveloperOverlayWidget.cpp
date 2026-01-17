// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolsDeveloperOverlayWidget.h"

#include "NActorPoolObject.h"
#include "NActorPoolSubsystem.h"
#include "Components/NListView.h"

void UNActorPoolsDeveloperOverlayWidget::NativeConstruct()
{
	AddWorldDelegateHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UNActorPoolsDeveloperOverlayWidget::OnWorldPostInitialization);
	RemoveWorldDelegateHandle = FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &UNActorPoolsDeveloperOverlayWidget::OnWorldBeginTearDown);
	
	Bind(GetWorld());
	
	Super::NativeConstruct();
}

void UNActorPoolsDeveloperOverlayWidget::NativeDestruct()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(AddWorldDelegateHandle);
	FWorldDelegates::OnWorldBeginTearDown.Remove(RemoveWorldDelegateHandle);
	
	Unbind(GetWorld());
	ActorPoolList->ClearListItems();
	
	Super::NativeDestruct();
}

void UNActorPoolsDeveloperOverlayWidget::Bind(UWorld* World)
{
	UNActorPoolSubsystem* System = UNActorPoolSubsystem::Get(World);
	if (System == nullptr) return; // System-less world
	
	// Build out known list
	TArray<FNActorPool*> KnownPools = System->GetAllPools();
	for (const auto Pool : KnownPools)
	{
		CreateListItem(Pool);
	}
	
	// Add delegate for future pools
	WorldToHandleMap.Add(World,System->OnActorPoolAdded.AddLambda([this] (FNActorPool* ActorPool)
	{
		this->CreateListItem(ActorPool);
	}));
}

void UNActorPoolsDeveloperOverlayWidget::Unbind(UWorld* World)
{
	UNActorPoolSubsystem* System = UNActorPoolSubsystem::Get(World);
	if (WorldToHandleMap.Contains(World))
	{
		System->OnActorPoolAdded.Remove(WorldToHandleMap[World]);
	}
	
	TArray<UObject*> Items = ActorPoolList->GetListItems();
	const int ItemCount = Items.Num();
	for (int i = ItemCount - 1; i >= 0; i--)
	{
		UNActorPoolObject* Object = Cast<UNActorPoolObject>(Items[i]);
		if (Object->GetPoolWorld() == World)
		{
			ActorPoolList->RemoveItem(Object);
			// 	Object->RemoveFromRoot();
		}

	}
}

void UNActorPoolsDeveloperOverlayWidget::OnWorldPostInitialization(UWorld* World,
	FWorldInitializationValues WorldInitializationValues)
{
	Bind(World);
}

void UNActorPoolsDeveloperOverlayWidget::OnWorldBeginTearDown(UWorld* World)
{
	Unbind(World);
}

void UNActorPoolsDeveloperOverlayWidget::CreateListItem(FNActorPool* ActorPool)
{
	UNActorPoolObject* Object = NewObject<UNActorPoolObject>(this, UNActorPoolObject::StaticClass(), NAME_None, RF_Transient);
	Object->Link(ActorPool);
	//Object->AddToRoot();
	ActorPoolList->AddItem(Object);
}
