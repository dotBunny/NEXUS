// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NActorPoolsDeveloperOverlay.h"

#include "NActorPoolListViewEntry.h"
#include "NActorPoolObject.h"
#include "NActorPoolSubsystem.h"
#include "NStyleLibrary.h"
#include "Components/NListView.h"
#include "Macros/NValidationMacros.h"

namespace NEXUS::ActorPools::ConsoleCommands
{
	static float DeveloperOverlayUpdateRate = 0.5f;
	static FAutoConsoleVariableRef CVAR_DeveloperOverlayUpdateRate(
		TEXT("N.ActorPools.DeveloperOverlay.UpdateRate"),
		DeveloperOverlayUpdateRate,
		TEXT("How often should the FNActorPools be pooled for updates."),
		ECVF_Default | ECVF_Preview);
}

void UNActorPoolsDeveloperOverlay::NativeConstruct()
{
	N_VALIDATE(LogNexusActorPools, ActorPoolList)
	
	AddWorldDelegateHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UNActorPoolsDeveloperOverlay::OnWorldPostInitialization);
	RemoveWorldDelegateHandle = FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &UNActorPoolsDeveloperOverlay::OnWorldBeginTearDown);
	
	BindAllCurrentWorlds();

	UpdateTimer = NEXUS::ActorPools::ConsoleCommands::DeveloperOverlayUpdateRate;

	UpdateBanner();
	
	Super::NativeConstruct();
}

void UNActorPoolsDeveloperOverlay::NativeDestruct()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(AddWorldDelegateHandle);
	FWorldDelegates::OnWorldBeginTearDown.Remove(RemoveWorldDelegateHandle);
	
	UnbindAllCurrentWorlds();

	ActorPoolList->ClearListItems();
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
	UpdateTimer = NEXUS::ActorPools::ConsoleCommands::DeveloperOverlayUpdateRate;
	
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

void UNActorPoolsDeveloperOverlay::BindWorld(UWorld* World)
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

void UNActorPoolsDeveloperOverlay::UnbindWorld(const UWorld* World)
{
	if (World == nullptr) return;
	
	UNActorPoolSubsystem* System = UNActorPoolSubsystem::Get(World);
	
	if (OnActorPoolAddedDelegates.Contains(World) && System != nullptr)
	{
		System->OnActorPoolAdded.Remove(OnActorPoolAddedDelegates[World]);
	}
	
	
	if (IsValid(ActorPoolList))
	{
		TArray<UObject*> Items = ActorPoolList->GetListItems();
		const int32 ItemCount = Items.Num();
		for (int32 i = ItemCount - 1; i >= 0; i--)
		{
			UNActorPoolObject* Object = Cast<UNActorPoolObject>(Items[i]);
			if (!IsValid(Object)) continue;
			
			if (Object->GetPoolWorld() == World)
			{
				ActorPoolList->RemoveItem(Object);
				
				// Remove our created object
				Object->MarkAsGarbage();
			}
		}
		UpdateBanner();
	}
}

void UNActorPoolsDeveloperOverlay::OnWorldPostInitialization(UWorld* World, FWorldInitializationValues WorldInitializationValues)
{
	BindWorld(World);
}

void UNActorPoolsDeveloperOverlay::OnWorldBeginTearDown(UWorld* World)
{
	if (World != nullptr)
	{
		UnbindWorld(World);
	}
}

void UNActorPoolsDeveloperOverlay::UpdateBanner() const
{
	if (IsValid(ActorPoolList) && ActorPoolList->GetNumItems() > 0)
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
	UNActorPoolObject* Object = UNActorPoolObject::Create(this, ActorPool);
	if (!ActorPoolList->GetListItems().Contains(Object))
	{
		ActorPoolList->AddItem(Object);
	}
}