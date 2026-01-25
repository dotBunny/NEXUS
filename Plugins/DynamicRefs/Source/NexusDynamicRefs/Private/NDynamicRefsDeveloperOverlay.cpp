// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefsDeveloperOverlay.h"

#include "NDynamicRefObject.h"
#include "NDynamicRefSubsystem.h"
#include "Components/NListView.h"

void UNDynamicRefsDeveloperOverlay::NativeConstruct()
{
	AddWorldDelegateHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UNDynamicRefsDeveloperOverlay::OnWorldPostInitialization);
	RemoveWorldDelegateHandle = FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &UNDynamicRefsDeveloperOverlay::OnWorldBeginTearDown);
	
	Bind(GetWorld());

	Super::NativeConstruct();
}

void UNDynamicRefsDeveloperOverlay::NativeDestruct()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(AddWorldDelegateHandle);
	FWorldDelegates::OnWorldBeginTearDown.Remove(RemoveWorldDelegateHandle);
	
	const UWorld* World = GetWorld();
	if (World != nullptr)
	{
		Unbind(World);
	}
	
	Super::NativeDestruct();
}

void UNDynamicRefsDeveloperOverlay::Bind(UWorld* World)
{
	UNDynamicRefSubsystem* System = UNDynamicRefSubsystem::Get(World);
	if (System == nullptr) return; // System-less world
	
	// check if entry exists
	// add new / add item too 
	
	
	
	// Get this worlds used references currently
	TArray<ENDynamicRef> DynamicRefs = System->GetDynamicRefs();
	for (ENDynamicRef DynamicRef : DynamicRefs)
	{
		if (DynamicRefObjects.Contains(DynamicRef))
		{
			
		}
		else
		{
			// make new
		}
		// ADD / CREATE
		//UNDynamicRefObject::Create(World, FName("Test"), World);
	}
	TArray<FName> Names = System->GetNames();
	for (FName Name : Names)
	{
		
		// Itterate over objects
		for (UObject* Object : System->GetObjectsByName(Name))
		{
			
		}
		if (NamedObjects.Contains(Name))
		{
			//NamedObjects[Name]->TargetObjects.Add()
		}
		else
		{
			// make new
		}
		// ADD / CREATE
		//UNDynamicRefObject::Create(World, FName("Test"), World);
	}
	
	// We've bound a world check it
	UpdateBanner();
	
	//
	// // Add delegate for future pools
	// OnActorPoolAddedDelegates.Add(World,System->OnActorPoolAdded.AddLambda([this] (FNActorPool* ActorPool)
	// {
	// 	this->CreateListItem(ActorPool);
	// 	this->UpdateBanner();
	// }));
}

void UNDynamicRefsDeveloperOverlay::Unbind(const UWorld* World, bool bClearItems)
{
	// UNActorPoolSubsystem* System = UNActorPoolSubsystem::Get(World);
	// if (OnActorPoolAddedDelegates.Contains(World) && System != nullptr)
	// {
	// 	System->OnActorPoolAdded.Remove(OnActorPoolAddedDelegates[World]);
	// }
	//
	// if (ActorPoolList != nullptr && ActorPoolList->IsValidLowLevel())
	// {
	// 	TArray<UObject*> Items = ActorPoolList->GetListItems();
	// 	const int ItemCount = Items.Num();
	// 	for (int i = ItemCount - 1; i >= 0; i--)
	// 	{
	// 		UNActorPoolObject* Object = Cast<UNActorPoolObject>(Items[i]);
	// 		if (Object->GetPoolWorld() == World)
	// 		{
	// 			ActorPoolList->RemoveItem(Object);
	// 		}
	// 	}
	// 	UpdateBanner();
	// }
}

void UNDynamicRefsDeveloperOverlay::OnWorldPostInitialization(UWorld* World,
	FWorldInitializationValues WorldInitializationValues)
{
	Bind(World);
}

void UNDynamicRefsDeveloperOverlay::OnWorldBeginTearDown(UWorld* World)
{
	if (World != nullptr)
	{
		Unbind(World);
	}
}

void UNDynamicRefsDeveloperOverlay::UpdateBanner() const
{
	if (References != nullptr && References->IsValidLowLevel() && References->GetNumItems() > 0)
	{
		HideBanner();
	}
	else
	{
		ShowBannerMessage();
	}
}
