#include "NProcGenOperationListViewEntry.h"

#include "CommonTextBlock.h"
#include "NProcGenOperation.h"
#include "Components/ProgressBar.h"

void UNProcGenOperationListViewEntry::NativeDestruct()
{
	Operation = nullptr;
	Super::NativeDestruct();
}

void UNProcGenOperationListViewEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	INListViewEntry::NativeOnListItemObjectSet(ListItemObject);
	Operation = Cast<UNProcGenOperation>(ListItemObject);
	
	Reset();
}

void UNProcGenOperationListViewEntry::Reset() const
{
	ProgressBar->SetPercent(0.f);
	if (Operation != nullptr)
	{
		LeftText->SetText(Operation->GetDisplayName());
		RightText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), 0, 0)));
		CenterText->SetText(FText::GetEmpty());
	}
	else
	{
		LeftText->SetText(FText::GetEmpty());
		RightText->SetText(FText::GetEmpty());
		CenterText->SetText(FText::GetEmpty());
	}
}
