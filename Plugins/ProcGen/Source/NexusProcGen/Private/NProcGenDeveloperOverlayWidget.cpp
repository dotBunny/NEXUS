// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenDeveloperOverlayWidget.h"

#include "NProcGenRegistry.h"
#include "Components/NListView.h"

void UNProcGenDeveloperOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Bind to Registry
	for (TArray<UNProcGenOperation*>& Operations = FNProcGenRegistry::GetOperations(); 
		UNProcGenOperation* Operation : Operations)
	{
		// Do not show EditorMode stuff
		if (!ShouldShowOperation(Operation->GetFName())) continue;
		OperationsList->AddItem(Cast<UObject>(Operation));
	}
	
	UpdateBanner();
	
	OperationsStatusChangedDelegateHandle = FNProcGenRegistry::OnOperationStateChanged.AddUObject(
		this, &UNProcGenDeveloperOverlayWidget::OnOperationStatusChanged);
}

void UNProcGenDeveloperOverlayWidget::NativeDestruct()
{
	if (OperationsList != nullptr && OperationsList->IsValidLowLevel())
	{
		OperationsList->ClearListItems();
	}
	
	FNProcGenRegistry::OnOperationStateChanged.Remove(OperationsStatusChangedDelegateHandle);
	
	Super::NativeDestruct();
}

void UNProcGenDeveloperOverlayWidget::OnOperationStatusChanged(UNProcGenOperation* Operation,
                                                               const ENProcGenOperationState NewState)
{
	if (!ShouldShowOperation(Operation->GetFName())) return;
	
	switch (NewState)
	{
		using enum ENProcGenOperationState;
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

void UNProcGenDeveloperOverlayWidget::UpdateBanner() const
{
	if (OperationsList != nullptr && OperationsList->IsValidLowLevel() && OperationsList->GetNumItems() > 0)
	{
		HideBanner();
	}
	else
	{
		ShowBannerMessage();
	}
}

bool UNProcGenDeveloperOverlayWidget::ShouldShowOperation(const FName& OperationName)
{
	if (OperationName == NEXUS::ProcGen::Operations::EditorMode) return false;
	return true;
}
