#include "NActorPoolListViewEntry.h"

#include "CommonTextBlock.h"
#include "Components/ProgressBar.h"

void UNActorPoolListViewEntry::NativeDestruct()
{
	Super::NativeDestruct();
}

void UNActorPoolListViewEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	UNActorPoolObject* ListPool = Cast<UNActorPoolObject>(ListItemObject);
	if (ListPool != nullptr)
	{
		Pool = ListPool;
		CenterText->SetText(Pool->GetClassName());
		Refresh();
	}
	else
	{
		CenterText->SetText(FText::GetEmpty());
	}
	INListViewEntry::NativeOnListItemObjectSet(ListItemObject);
}

void UNActorPoolListViewEntry::Refresh()
{
	if (Pool == nullptr)
	{
		return;
	}
	const int InCount = Pool->GetInCount();
	if (InCount == 0)
	{
		return;
	}
	const int OutCount = Pool->GetOutCount();
	const int Total = InCount + OutCount;
	
	LeftText->SetText(FText::AsNumber(InCount));
	LeftText->SetText(FText::AsNumber(OutCount));
	ProgressBar->SetPercent(static_cast<float>(InCount)/Total);
}

void UNActorPoolListViewEntry::Reset() const
{
	LeftText->SetText(FText::GetEmpty());
	RightText->SetText(FText::GetEmpty());
	CenterText->SetText(FText::GetEmpty());
	ProgressBar->SetPercent(0.f);
}
