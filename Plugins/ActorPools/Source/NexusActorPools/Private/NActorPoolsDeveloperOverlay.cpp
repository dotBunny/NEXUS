// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolsDeveloperOverlay.h"

#include "NActorPoolListViewEntry.h"
#include "NActorPoolObject.h"
#include "NActorPoolSubsystem.h"
#include "NStyleLibrary.h"
#include "Components/NListView.h"

void UNActorPoolsDeveloperOverlay::NativeConstruct()
{
	AddWorldDelegateHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UNActorPoolsDeveloperOverlay::OnWorldPostInitialization);
	RemoveWorldDelegateHandle = FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &UNActorPoolsDeveloperOverlay::OnWorldBeginTearDown);
	
	// Look at all worlds and add them to bindings
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const auto& Context : WorldContexts)
	{
		Bind(Context.World());
	}
	
	const UNActorPoolsSettings* Settings = UNActorPoolsSettings::Get();
	CachedUpdateRate = Settings->DeveloperOverlayUpdateRate;
	UpdateTimer = CachedUpdateRate;

	UpdateBanner();
	
	Super::NativeConstruct();
}

void UNActorPoolsDeveloperOverlay::NativeDestruct()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(AddWorldDelegateHandle);
	FWorldDelegates::OnWorldBeginTearDown.Remove(RemoveWorldDelegateHandle);
	
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const auto& Context : WorldContexts)
	{
		Unbind(Context.World());
	}
	
	Super::NativeDestruct();
}

void UNActorPoolsDeveloperOverlay::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
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

void UNActorPoolsDeveloperOverlay::Bind(UWorld* World)
{
	if (World == nullptr) return;
	
	UNActorPoolSubsystem* System = UNActorPoolSubsystem::Get(World);
	if (System == nullptr)
	{
		return; // System-less world
	}
	
	// Build out known list
	TArray<FNActorPool*> KnownPools = System->GetAllPools();
	for (const auto Pool : KnownPools)
	{
		CreateListItem(Pool);
	}

	// We've bound a world check it
	UpdateBanner();
	
	// Add delegate for future pools
	OnActorPoolAddedDelegates.Add(World,System->OnActorPoolAdded.AddLambda([this] (FNActorPool* ActorPool)
	{
		this->CreateListItem(ActorPool);
		this->UpdateBanner();
	}));
}

void UNActorPoolsDeveloperOverlay::Unbind(const UWorld* World)
{
	if (World == nullptr) return;
	
	UNActorPoolSubsystem* System = UNActorPoolSubsystem::Get(World);
	if (OnActorPoolAddedDelegates.Contains(World) && System != nullptr)
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
		UpdateBanner();
	}
}

void UNActorPoolsDeveloperOverlay::OnWorldPostInitialization(UWorld* World, 
                                                                   FWorldInitializationValues WorldInitializationValues)
{
	Bind(World);
}

void UNActorPoolsDeveloperOverlay::OnWorldBeginTearDown(UWorld* World)
{
	if (World != nullptr)
	{
		Unbind(World);
	}
}

void UNActorPoolsDeveloperOverlay::UpdateBanner() const
{
	if (ActorPoolList != nullptr && ActorPoolList->IsValidLowLevel() && ActorPoolList->GetNumItems() > 0)
	{
		HideContainerBanner();
	}
	else
	{
		ShowContainerBanner(NoActorPoolsFoundText,
			UNStyleLibrary::GetInfoForegroundColor(), 
			UNStyleLibrary::GetInfoBackgroundColor());
	}
}

void UNActorPoolsDeveloperOverlay::CreateListItem(FNActorPool* ActorPool)
{
	ActorPoolList->AddItem(UNActorPoolObject::Create(this, ActorPool));
}