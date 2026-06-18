// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Widgets/NTextListViewEntry.h"

#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Macros/NValidationMacros.h"

void UNTextListViewEntry::NativeConstruct()
{
	Super::NativeConstruct();

	// Will validate it here only to throw a message in log for someone to realize they haven't hooked up the widget correctly.
	N_VALIDATE(LogNexusUI, Text);
	N_VALIDATE(LogNexusUI, Container);
}

void UNTextListViewEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	const UNTextListEntry* TextObject = Cast<UNTextListEntry>(ListItemObject);
	if (TextObject != nullptr)
	{
		SetText(TextObject->GetText());

		if (TextObject->HasForegroundColor())
		{
			SetTextColor(TextObject->GetForegroundColor());
		}
		if (TextObject->HasBackgroundColor())
		{
			SetBackgroundColor(TextObject->GetBackgroundColor());
		}
	}
}

void UNTextListViewEntry::SetOwnerListView(UObject* Widget, UNListView* Owner)
{
	OwnerListView = Owner;
	INListViewEntry::SetOwnerListView(Widget, Owner);
}

void UNTextListViewEntry::SetText(const FText NewText) const
{
	Text->SetText(NewText);
}

void UNTextListViewEntry::SetTextColor(ENColor NewColor) const
{
	Text->SetColorAndOpacity(FNColor::GetLinearColor(NewColor));
}

void UNTextListViewEntry::SetBackgroundColor(ENColor NewColor) const
{
	Container->SetBrushColor(FNColor::GetLinearColor(NewColor));
}
