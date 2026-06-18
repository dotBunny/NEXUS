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
	// BindWidget pointers are required bindings (non-null in a compiling Blueprint); guard anyway so a
	// malformed cooked subclass missing a widget fails soft rather than crashing on the deref.
	if (!IsValid(Data) || !LeftText || !CenterText || !RightText || !ProgressBar)
	{
		return;
	}

	LeftText->SetText(FText::FromString(Data->GetLabel()));
	CenterText->SetText(FText::FromString(Data->GetMessage()));

	// OnChanged is a single coarse broadcast (label/message/percent share it), so this runs on label-only
	// changes too. Skip the percent SetText — and the Slate relayout it forces — unless the rounded value moved.
	const int32 Percent = FMath::RoundToInt(Data->GetPercent() * 100.f);
	if (Percent != LastShownPercent)
	{
		LastShownPercent = Percent;
		RightText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), Percent)));
	}

	ProgressBar->SetPercent(Data->GetPercent());
}

void UNProgressBarListViewEntry::Reset() const
{
	if (!LeftText || !CenterText || !RightText || !ProgressBar)
	{
		return;
	}

	LeftText->SetText(FText::GetEmpty());
	CenterText->SetText(FText::GetEmpty());
	RightText->SetText(FText::GetEmpty());
	ProgressBar->SetPercent(0.f);

	// Clearing RightText above invalidates the cache; reseed so a recycled entry repaints even if its
	// new data happens to sit at the same rounded percent.
	LastShownPercent = MIN_int32;
}
