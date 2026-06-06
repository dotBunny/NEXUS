// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Assembly/NAssemblyOperationListViewEntry.h"

#include "CommonTextBlock.h"
#include "Assembly/NAssemblyOperation.h"
#include "Components/ProgressBar.h"

void UNAssemblyOperationListViewEntry::NativeDestruct()
{
	if (IsValid(Operation))
	{
		Operation->OnStatusMessageChanged.RemoveDynamic(this, &UNAssemblyOperationListViewEntry::OnOperationDisplayMessageChanged);\
		Operation->OnTasksChanged.RemoveDynamic(this, &UNAssemblyOperationListViewEntry::OnOperationTasksChanged);
	}
	Operation = nullptr;
	Super::NativeDestruct();
}

void UNAssemblyOperationListViewEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	INListViewEntry::NativeOnListItemObjectSet(ListItemObject);
	Operation = Cast<UNAssemblyOperation>(ListItemObject);
	Operation->OnStatusMessageChanged.AddDynamic(this, &UNAssemblyOperationListViewEntry::OnOperationDisplayMessageChanged);
	Operation->OnTasksChanged.AddDynamic(this, &UNAssemblyOperationListViewEntry::OnOperationTasksChanged);
	Reset();
}

void UNAssemblyOperationListViewEntry::Reset() const
{
	if (IsValid(Operation))
	{
		LeftText->SetText(Operation->GetDisplayName());
		const FIntVector2 Tasks = Operation->GetCachedTaskStatusCounts();
		CenterText->SetText(FText::FromString(Operation->GetDisplayMessage()));

		if (Tasks.Y != 0)
		{
			RightText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), Tasks.X, Tasks.Y)));
			ProgressBar->SetPercent((static_cast<float>(Tasks.X)/Tasks.Y));
		}
		else
		{
			RightText->SetText(FText::GetEmpty());
			ProgressBar->SetPercent(1.f);
		}
	}
	else
	{
		LeftText->SetText(FText::GetEmpty());
		RightText->SetText(FText::GetEmpty());
		
		CenterText->SetText(FText::GetEmpty());
		ProgressBar->SetPercent(1.f);
	}
	
	UpdateCancelButtonVisibility();
}

void UNAssemblyOperationListViewEntry::OnOperationDisplayMessageChanged(const FString& NewDisplayMessage)
{
	CenterText->SetText(FText::FromString(NewDisplayMessage));
	UpdateCancelButtonVisibility();
}

void UNAssemblyOperationListViewEntry::OnOperationTasksChanged(const int32 CompletedTasks, const int32 TotalTasks)
{
	if (TotalTasks != 0)
	{
		RightText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), CompletedTasks, TotalTasks)));
		ProgressBar->SetPercent(static_cast<float>(CompletedTasks)/TotalTasks);
	}
	else
	{
		RightText->SetText(FText::GetEmpty());
		ProgressBar->SetPercent(1.f);
	}
	
	UpdateCancelButtonVisibility();
}

void UNAssemblyOperationListViewEntry::OnCancelButtonClicked()
{
	if (IsValid(Operation))
	{
		Operation->Cancel();
	}
}

void UNAssemblyOperationListViewEntry::UpdateCancelButtonVisibility() const
{
	if (IsValid(Operation))
	{
		if (Operation->IsRunning())
		{
			CancelButton->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			CancelButton->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else
	{
		CancelButton->SetVisibility(ESlateVisibility::Hidden);
	}
}
