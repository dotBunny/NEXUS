// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NDynamicRefsDeveloperOverlay.h"

#include "NDynamicRefObject.h"
#include "NDynamicRefSubsystem.h"
#include "NStyleLibrary.h"
#include "Components/NListView.h"

void UNDynamicRefsDeveloperOverlay::NativeConstruct()
{
	Super::NativeConstruct();
	
	N_VALIDATE(LogNexusDynamicRefs, DynamicReferencesHeader)
	N_VALIDATE(LogNexusDynamicRefs, DynamicReferences)
	N_VALIDATE(LogNexusDynamicRefs, NamedReferencesHeader)
	N_VALIDATE(LogNexusDynamicRefs, NamedReferences)
}

void UNDynamicRefsDeveloperOverlay::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	ReconcileStaleEntries();
}

void UNDynamicRefsDeveloperOverlay::ReconcileStaleEntries()
{
	bool bChanged = false;

	// Objects removed via the subsystem fire OnRemoved and are handled by RemoveListItem; objects destroyed
	// without a matching remove go stale silently, so sweep the wrappers and drop any that are now empty.
	// UNDynamicRefObject::GetCount() prunes stale weak entries first, so == 0 means every tracked object is gone.
	for (auto It = DynamicRefObjects.CreateIterator(); It; ++It)
	{
		if (It.Value()->GetCount() == 0)
		{
			DynamicReferences->RemoveItem(It.Value());
			It.RemoveCurrent();
			bChanged = true;
		}
	}

	for (auto It = NamedObjects.CreateIterator(); It; ++It)
	{
		if (It.Value()->GetCount() == 0)
		{
			NamedReferences->RemoveItem(It.Value());
			It.RemoveCurrent();
			bChanged = true;
		}
	}

	if (bChanged)
	{
		UpdateBanner();
	}
}

void UNDynamicRefsDeveloperOverlay::BindWorld(UWorld* World)
{
	if (World == nullptr) return;
	
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
			DynamicReferences->AddItem(DynamicRefObjects.Add(DynamicRef, UNDynamicRefObject::Create(this, DynamicRef)));
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



void UNDynamicRefsDeveloperOverlay::UnbindWorld(const UWorld* World)
{
	if (World == nullptr) return;
	
	UNDynamicRefSubsystem* System = UNDynamicRefSubsystem::Get(World);
	
	// OnAddedDelegates
	if (OnAddedDelegates.Contains(World))
	{
		if (System != nullptr)
		{
			System->OnAdded.Remove(OnAddedDelegates[World]);
		}
		OnAddedDelegates.Remove(World);
	}
	
	// OnAddedByNameDelegates
	if (OnAddedByNameDelegates.Contains(World))
	{
		if (System != nullptr)
		{
			System->OnAddedByName.Remove(OnAddedByNameDelegates[World]);
		}
		OnAddedByNameDelegates.Remove(World);
	}
	
	// OnRemovedDelegates
	if (OnRemovedDelegates.Contains(World))
	{
		if (System != nullptr)
		{
			System->OnRemoved.Remove(OnRemovedDelegates[World]);
		}
		OnRemovedDelegates.Remove(World);
	}
	
	// OnRemovedByNameDelegates
	if (OnRemovedByNameDelegates.Contains(World))
	{
		if (System != nullptr)
		{
			System->OnRemovedByName.Remove(OnRemovedByNameDelegates[World]);
		}
		OnRemovedByNameDelegates.Remove(World);
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

void UNDynamicRefsDeveloperOverlay::UpdateBanner() const
{
	const bool bNamedReferences = (IsValid(NamedReferences) &&
		NamedReferences->GetNumItems() > 0);
	const bool bDynamicReferences = (IsValid(DynamicReferences) &&
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
