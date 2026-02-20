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
		
		
		// Pool Name
		CenterText->SetText(Pool->GetClassName());
		
		// Handle Color
		if (Pool->DoesSupportInterface())
		{
			TypeImage->SetBrushTintColor(FLinearColor::Green);
			TypeImage->SetToolTipText(FText::FromString("INActorPoolItem"));
		}
		else if (Pool->HasInvokeUFunctionFlag())
		{
			TypeImage->SetBrushTintColor(FLinearColor::Yellow);
			TypeImage->SetToolTipText(FText::FromString("Invoke UFUNCTION"));
		}
		else
		{
			TypeImage->SetBrushTintColor(FLinearColor::Blue);
			TypeImage->SetToolTipText(FText::FromString("No Callbacks"));
		}
		
		if (Pool->GetWorld() != nullptr)
		{
			ProgressBar->SetToolTipText(FText::FromString(Pool->GetWorld()->GetName()));
		}
	
		Refresh();
	}
	else
	{
		TypeImage->SetBrushTintColor(FLinearColor::Gray);
		TypeImage->SetToolTipText(FText::GetEmpty());
		CenterText->SetText(FText::GetEmpty());
	}
	INListViewEntry::NativeOnListItemObjectSet(ListItemObject);
}

void UNActorPoolListViewEntry::Refresh() const
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
	RightText->SetText(FText::Format(NSLOCTEXT("NexusActorPools", "OutAndTotal", "{0} | {1}"), FText::AsNumber(OutCount), FText::AsNumber(Total)));
	
	ProgressBar->SetPercent(static_cast<float>(InCount)/Total);
}

void UNActorPoolListViewEntry::Reset() const
{
	LeftText->SetText(FText::GetEmpty());
	RightText->SetText(FText::GetEmpty());
	CenterText->SetText(FText::GetEmpty());
	ProgressBar->SetPercent(0.f);
}
