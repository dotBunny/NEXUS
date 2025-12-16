#include "NProcGenOperationListViewEntry.h"

#include "CommonTextBlock.h"
#include "NProcGenOperation.h"
#include "Components/ProgressBar.h"

void UNProcGenOperationListViewEntry::NativeDestruct()
{
	if (Operation != nullptr && Operation->IsValidLowLevel())
	{
		Operation->OnDisplayMessageChanged.RemoveDynamic(this, &UNProcGenOperationListViewEntry::OnOperationDisplayMessageChanged);
	}
	Operation = nullptr;
	Super::NativeDestruct();
}

void UNProcGenOperationListViewEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	INListViewEntry::NativeOnListItemObjectSet(ListItemObject);
	Operation = Cast<UNProcGenOperation>(ListItemObject);
	Operation->OnDisplayMessageChanged.AddDynamic(this, &UNProcGenOperationListViewEntry::OnOperationDisplayMessageChanged);
	Reset();
}

void UNProcGenOperationListViewEntry::Reset() const
{
	ProgressBar->SetPercent(0.f);
	
	if (Operation != nullptr)
	{
		LeftText->SetText(Operation->GetDisplayName());
		RightText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), 0, 0))); // Todo tasks?
		CenterText->SetText(FText::FromString(Operation->GetDisplayMessage()));
	}
	else
	{
		LeftText->SetText(FText::GetEmpty());
		RightText->SetText(FText::GetEmpty());
		
		CenterText->SetText(FText::GetEmpty());
	}
}

void UNProcGenOperationListViewEntry::OnOperationDisplayMessageChanged(const FString& NewDisplayMessage)
{
	CenterText->SetText(FText::FromString(NewDisplayMessage));
}
