// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "NProcGenDeveloperOverlayWidget.h"

#include "NProcGenRegistry.h"
#include "ParticleHelper.h"
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
	
	OperationsStatusChangedDelegateHandle = FNProcGenRegistry::OnOperationStateChanged.AddUObject(
		this, &UNProcGenDeveloperOverlayWidget::OnOperationStatusChanged);
}

void UNProcGenDeveloperOverlayWidget::NativeDestruct()
{
	OperationsList->ClearListItems();
	
	FNProcGenRegistry::OnOperationStateChanged.Remove(OperationsStatusChangedDelegateHandle);
	
	Super::NativeDestruct();
}

void UNProcGenDeveloperOverlayWidget::OnOperationStatusChanged(UNProcGenOperation* Operation,
	const ENProcGenOperationState NewState)
{
	if (!ShouldShowOperation(Operation->GetFName())) return;
	
	switch (NewState)
	{
	case PGOS_None:
		break;
	case PGOS_Registered:
		OperationsList->AddItem(Cast<UObject>(Operation));
		break;
	case PGOS_Started:
		break;
	case PGOS_Updated:
		// No tick!
		break;
	case PGOS_Finished:
		break;
	case PGOS_Unregistered:
		OperationsList->RemoveItem(Cast<UObject>(Operation));
		break;
	}
}

bool UNProcGenDeveloperOverlayWidget::ShouldShowOperation(const FName& OperationName)
{
	if (OperationName == NEXUS::ProcGen::Operations::EditorMode) return false;
	return true;
}
