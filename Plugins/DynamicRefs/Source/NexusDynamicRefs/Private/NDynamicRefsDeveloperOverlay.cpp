// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefsDeveloperOverlay.h"

#include "NDynamicRefObject.h"
#include "NDynamicRefSubsystem.h"
#include "NStyleLibrary.h"
#include "Components/NListView.h"

void UNDynamicRefsDeveloperOverlay::NativeConstruct()
{
	AddWorldDelegateHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UNDynamicRefsDeveloperOverlay::OnWorldPostInitialization);
	RemoveWorldDelegateHandle = FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &UNDynamicRefsDeveloperOverlay::OnWorldBeginTearDown);
	
	// TODO: Binding of existing work when adding to existing playmode
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
	if (System == nullptr)
	{
		UpdateBanner();
		return; // System-less world
	}
	
	// Get this worlds used references currently
	TArray<ENDynamicRef> DynamicRefs = System->GetDynamicRefs();
	for (ENDynamicRef DynamicRef : DynamicRefs)
	{
		if (!DynamicRefObjects.Contains(DynamicRef))
		{
			NamedReferences->AddItem(DynamicRefObjects.Add(DynamicRef, UNDynamicRefObject::Create(this, DynamicRef)));
		}
		for (UObject* Object : System->GetObjects(DynamicRef))
		{
			DynamicRefObjects[DynamicRef]->AddObject(Object);
		}
	}
	
	TArray<FName> Names = System->GetNames();
	for (FName Name : Names)
	{
		if (!NamedObjects.Contains(Name))
		{
			NamedReferences->AddItem(NamedObjects.Add(Name, UNDynamicRefObject::Create(this, Name)));
		}
		for (UObject* Object : System->GetObjectsByName(Name))
		{
			NamedObjects[Name]->AddObject(Object);
		}
	}
	
	// We've bound a world check it
	UpdateBanner();
	

	// Add delegate for future pools
	OnAddedDelegates.Add(World,System->OnAdded.AddLambda([this] (ENDynamicRef DynamicRef, UObject* Object)
	{
		this->AddListItem(DynamicRef, Object);
		this->UpdateBanner();
	}));
	OnAddedByNameDelegates.Add(World,System->OnAddedByName.AddLambda([this] (FName Name, UObject* Object)
	{
		this->AddListItem(Name, Object);
		this->UpdateBanner();
	}));
	OnRemovedDelegates.Add(World,System->OnRemoved.AddLambda([this] (ENDynamicRef DynamicRef, UObject* Object)
	{
		this->RemoveListItem(DynamicRef, Object);
		this->UpdateBanner();
	}));
	OnRemovedByNameDelegates.Add(World,System->OnRemovedByName.AddLambda([this] (FName Name, UObject* Object)
	{
		this->RemoveListItem(Name, Object);
		this->UpdateBanner();
	}));
}



void UNDynamicRefsDeveloperOverlay::Unbind(const UWorld* World, bool bClearItems)
{
	UNDynamicRefSubsystem* System = UNDynamicRefSubsystem::Get(World);
	if (OnAddedDelegates.Contains(World) && System != nullptr)
	{
		System->OnAdded.Remove(OnAddedDelegates[World]);
	}
	if (OnAddedByNameDelegates.Contains(World) && System != nullptr)
	{
		System->OnAddedByName.Remove(OnAddedByNameDelegates[World]);
	}
	if (OnRemovedDelegates.Contains(World) && System != nullptr)
	{
		System->OnRemoved.Remove(OnRemovedDelegates[World]);
	}
	if (OnRemovedByNameDelegates.Contains(World) && System != nullptr)
	{
		System->OnRemovedByName.Remove(OnRemovedByNameDelegates[World]);
	}
	
	if (System != nullptr)
	{
		TArray<ENDynamicRef> DynamicRefs = System->GetDynamicRefs();
		for (ENDynamicRef DynamicRef : DynamicRefs)
		{
			for (UObject* Object : System->GetObjects(DynamicRef))
			{
				RemoveListItem(DynamicRef, Object);
			}
		}
	
		TArray<FName> Names = System->GetNames();
		for (FName Name : Names)
		{
			for (UObject* Object : System->GetObjectsByName(Name))
			{
				RemoveListItem(Name, Object);
			}
		}
	}
	
	UpdateBanner();
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
	const bool bNamedReferences = (NamedReferences != nullptr && 
		NamedReferences->IsValidLowLevel() && 
		NamedReferences->GetNumItems() > 0);
	const bool bDynamicReferences = (DynamicReferences != nullptr && 
		DynamicReferences->IsValidLowLevel() && 
		DynamicReferences->GetNumItems() > 0);
		
	if (bNamedReferences || bDynamicReferences)
	{
		// add check
		HideContainerBanner();
		
		if (bNamedReferences)
		{
			NamedReferencesHeader->SetVisibility(ESlateVisibility::Visible);
		}
		if (bDynamicReferences)
		{
			DynamicReferencesHeader->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		if (DynamicReferencesHeader != nullptr)
		{
			DynamicReferencesHeader->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (NamedReferencesHeader != nullptr)
		{
			NamedReferencesHeader->SetVisibility(ESlateVisibility::Collapsed);
		}
		
		ShowContainerBanner(NoReferencesFoundText,
	UNStyleLibrary::GetInfoForegroundColor(), 
	UNStyleLibrary::GetInfoBackgroundColor());
	}
}

void UNDynamicRefsDeveloperOverlay::AddListItem(FName Name, UObject* Object)
{
	if (!NamedObjects.Contains(Name))
	{
		NamedReferences->AddItem(NamedObjects.Add(Name, UNDynamicRefObject::Create(this, Name)));
	}
	NamedObjects[Name]->AddObject(Object);
}

void UNDynamicRefsDeveloperOverlay::AddListItem(ENDynamicRef DynamicRef, UObject* Object)
{
	if (!DynamicRefObjects.Contains(DynamicRef))
	{
		DynamicReferences->AddItem(DynamicRefObjects.Add(DynamicRef, UNDynamicRefObject::Create(this, DynamicRef)));
	}
	DynamicRefObjects[DynamicRef]->AddObject(Object);
}

void UNDynamicRefsDeveloperOverlay::RemoveListItem(FName Name, UObject* Object)
{
	if (NamedObjects.Contains(Name))
	{
		NamedObjects[Name]->RemoveObject(Object);
		if (NamedObjects[Name]->GetCount() == 0)
		{
			NamedReferences->RemoveItem(NamedObjects[Name]);
			NamedObjects.Remove(Name);
		}
	}
}

void UNDynamicRefsDeveloperOverlay::RemoveListItem(ENDynamicRef DynamicRef, UObject* Object)
{
	if (DynamicRefObjects.Contains(DynamicRef))
	{
		DynamicRefObjects[DynamicRef]->RemoveObject(Object);
		if (DynamicRefObjects[DynamicRef]->GetCount() == 0)
		{
			DynamicReferences->RemoveItem(DynamicRefObjects[DynamicRef]);
			DynamicRefObjects.Remove(DynamicRef);
		}
	}
}
