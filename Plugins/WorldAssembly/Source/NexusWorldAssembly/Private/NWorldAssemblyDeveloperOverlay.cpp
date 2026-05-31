// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NWorldAssemblyDeveloperOverlay.h"

#include "NWorldAssemblyRegistry.h"
#include "NStyleLibrary.h"
#include "Components/NListView.h"
#include "Macros/NValidationMacros.h"

void UNWorldAssemblyDeveloperOverlay::NativeConstruct()
{
	Super::NativeConstruct();
	
	N_VALIDATE(LogNexusWorldAssembly, OperationsList)
	
	// Bind to Registry
	for (TArray<UNAssemblyOperation*>& Operations = FNWorldAssemblyRegistry::GetOperations(); 
		UNAssemblyOperation* Operation : Operations)
	{
		// Do not show EditorMode stuff
		if (!ShouldShowOperation(Operation->GetFName())) continue;
		OperationsList->AddItem(Cast<UObject>(Operation));
	}
	
	UpdateBanner();
	
	OperationsStatusChangedDelegateHandle = FNWorldAssemblyRegistry::OnOperationStateChanged.AddUObject(
		this, &UNWorldAssemblyDeveloperOverlay::OnOperationStatusChanged);
}

void UNWorldAssemblyDeveloperOverlay::NativeDestruct()
{
	if (IsValid(OperationsList))
	{
		OperationsList->ClearListItems();
	}
	
	FNWorldAssemblyRegistry::OnOperationStateChanged.Remove(OperationsStatusChangedDelegateHandle);
	
	Super::NativeDestruct();
}

void UNWorldAssemblyDeveloperOverlay::OnOperationStatusChanged(UNAssemblyOperation* Operation,
                                                               const ENWorldAssemblyOperationState NewState)
{
	if (!ShouldShowOperation(Operation->GetFName())) return;
	
	switch (NewState)
	{
		using enum ENWorldAssemblyOperationState;
	case None:
		break;
	case Registered:
		OperationsList->AddItem(Cast<UObject>(Operation));
		UpdateBanner();
		break;
	case Started:
		break;
	case Updated:
		// No tick!
		break;
	case Finished:
		break;
	case Unregistered:
		OperationsList->RemoveItem(Cast<UObject>(Operation));
		UpdateBanner();
		break;
	}
}

void UNWorldAssemblyDeveloperOverlay::UpdateBanner() const
{
	if (IsValid(OperationsList) && OperationsList->GetNumItems() > 0)
	{
		HideContainerBanner();
	}
	else
	{
		ShowContainerBanner(NoOperationsFoundText,
			UNStyleLibrary::GetInfoForegroundColor(), 
			UNStyleLibrary::GetInfoBackgroundColor());
	}
}

bool UNWorldAssemblyDeveloperOverlay::ShouldShowOperation(const FName& OperationName)
{
	if (OperationName == NEXUS::WorldAssembly::Operations::EditorMode) return false;
	return true;
}
