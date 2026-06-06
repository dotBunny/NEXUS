// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Widgets/NProgressBarListViewEntry.h"

#include "CommonTextBlock.h"
#include "Components/ProgressBar.h"

void UNProgressBarListEntry::SetLabel(const FString& InLabel)
{
	if (!Label.Equals(InLabel))
	{
		Label = InLabel;
		OnChanged.Broadcast();
	}
}

void UNProgressBarListEntry::SetMessage(const FString& InMessage)
{
	if (!Message.Equals(InMessage))
	{
		Message = InMessage;
		OnChanged.Broadcast();
	}
}

void UNProgressBarListEntry::SetPercent(float InPercent)
{
	const float Clamped = FMath::Clamp(InPercent, 0.f, 1.f);
	if (Percent != Clamped)
	{
		Percent = Clamped;
		OnChanged.Broadcast();
	}
}

void UNProgressBarListEntry::SetStatus(const FString& InMessage, float InPercent)
{
	const float Clamped = FMath::Clamp(InPercent, 0.f, 1.f);
	if (!Message.Equals(InMessage) || Percent != Clamped)
	{
		Message = InMessage;
		Percent = Clamped;
		OnChanged.Broadcast();
	}
}

void UNProgressBarListViewEntry::NativeDestruct()
{
	if (IsValid(Data))
	{
		Data->OnChanged.RemoveDynamic(this, &UNProgressBarListViewEntry::OnDataChanged);
	}
	Data = nullptr;
	Super::NativeDestruct();
}

void UNProgressBarListViewEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	INListViewEntry::NativeOnListItemObjectSet(ListItemObject);

	// Drop any previous binding before rebinding to the newly assigned data object (entries are recycled).
	if (IsValid(Data))
	{
		Data->OnChanged.RemoveDynamic(this, &UNProgressBarListViewEntry::OnDataChanged);
	}

	Data = Cast<UNProgressBarListEntry>(ListItemObject);
	if (IsValid(Data))
	{
		Data->OnChanged.AddDynamic(this, &UNProgressBarListViewEntry::OnDataChanged);
		UpdateFromData();
	}
	else
	{
		Reset();
	}
}

void UNProgressBarListViewEntry::NativeOnEntryReleased()
{
	if (IsValid(Data))
	{
		Data->OnChanged.RemoveDynamic(this, &UNProgressBarListViewEntry::OnDataChanged);
	}
	Data = nullptr;
	Reset();
}

void UNProgressBarListViewEntry::OnDataChanged()
{
	UpdateFromData();
}

void UNProgressBarListViewEntry::UpdateFromData() const
{
	if (IsValid(Data))
	{
		LeftText->SetText(FText::FromString(Data->GetLabel()));
		CenterText->SetText(FText::FromString(Data->GetMessage()));
		RightText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Data->GetPercent() * 100.f))));
		ProgressBar->SetPercent(Data->GetPercent());
	}
}

void UNProgressBarListViewEntry::Reset() const
{
	LeftText->SetText(FText::GetEmpty());
	CenterText->SetText(FText::GetEmpty());
	RightText->SetText(FText::GetEmpty());
	ProgressBar->SetPercent(0.f);
}
