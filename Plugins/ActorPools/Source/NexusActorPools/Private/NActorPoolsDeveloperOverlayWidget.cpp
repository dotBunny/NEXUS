// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolsDeveloperOverlayWidget.h"

#include "NActorPoolListViewEntry.h"
#include "NActorPoolObject.h"
#include "NActorPoolSubsystem.h"
#include "Components/NListView.h"

void UNActorPoolsDeveloperOverlayWidget::NativeConstruct()
{
	AddWorldDelegateHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UNActorPoolsDeveloperOverlayWidget::OnWorldPostInitialization);
	RemoveWorldDelegateHandle = FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &UNActorPoolsDeveloperOverlayWidget::OnWorldBeginTearDown);
	
	Bind(GetWorld());
	
	const UNActorPoolsSettings* Settings = UNActorPoolsSettings::Get();
	CachedUpdateRate = Settings->DeveloperOverlayUpdateRate;
	UpdateTimer = CachedUpdateRate;

	Super::NativeConstruct();
}

void UNActorPoolsDeveloperOverlayWidget::NativeDestruct()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(AddWorldDelegateHandle);
	FWorldDelegates::OnWorldBeginTearDown.Remove(RemoveWorldDelegateHandle);
	
	Unbind(GetWorld());
	
	Super::NativeDestruct();
}

void UNActorPoolsDeveloperOverlayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	UpdateTimer -= InDeltaTime;
	if (UpdateTimer > 0.f)
	{
		return;
	}
	
	// Reset timer
	UpdateTimer = CachedUpdateRate;
	
	for (const auto Item : ActorPoolList->GetDisplayedEntryWidgets())
	{
		const UNActorPoolListViewEntry* Entry = Cast<UNActorPoolListViewEntry>(Item);
		if (Entry != nullptr)
		{
			Entry->Refresh();
		}
	}
	Super::NativeTick(MyGeometry, InDeltaTime);
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
	OnActorPoolAddedDelegates.Add(World,System->OnActorPoolAdded.AddLambda([this] (FNActorPool* ActorPool)
	{
		this->CreateListItem(ActorPool);
	}));
}

void UNActorPoolsDeveloperOverlayWidget::Unbind(const UWorld* World)
{
	UNActorPoolSubsystem* System = UNActorPoolSubsystem::Get(World);
	if (OnActorPoolAddedDelegates.Contains(World))
	{
		System->OnActorPoolAdded.Remove(OnActorPoolAddedDelegates[World]);
	}
	
	if (ActorPoolList != nullptr && ActorPoolList->IsValidLowLevel())
	{
		TArray<UObject*> Items = ActorPoolList->GetListItems();
		const int ItemCount = Items.Num();
		for (int i = ItemCount - 1; i >= 0; i--)
		{
			UNActorPoolObject* Object = Cast<UNActorPoolObject>(Items[i]);
			if (Object->GetPoolWorld() == World)
			{
				ActorPoolList->RemoveItem(Object);
			}
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
	ActorPoolList->AddItem(UNActorPoolObject::Create(this, ActorPool));
}
