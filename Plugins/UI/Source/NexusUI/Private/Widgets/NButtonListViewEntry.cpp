// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#include "Widgets/NButtonListViewEntry.h"

#include "CommonTextBlock.h"
#include "Components/Button.h"
#include "Macros/NValidationMacros.h"

void UNButtonListViewEntry::NativeConstruct()
{
	Super::NativeConstruct();

	// Will validate it here only to throw a message in log for someone to realize they havent hooked up the widget correctly.
	N_VALIDATE(LogNexusUI, Text);
	N_VALIDATE_RETURN_VOID(LogNexusUI, Button);


	// Pressed colors must show during the hold, so bind the color swap to OnPressed (mouse-down),
	// not OnClicked (which fires on release and would leave the pressed colors applied after the click).
	Button->OnClicked.AddDynamic(this, &UNButtonListViewEntry::HandleClicked);
	Button->OnPressed.AddDynamic(this, &UNButtonListViewEntry::OnButtonPressed);
	Button->OnHovered.AddDynamic(this, &UNButtonListViewEntry::OnButtonHovered);
	Button->OnUnhovered.AddDynamic(this, &UNButtonListViewEntry::OnButtonUnhovered);
	Button->OnReleased.AddDynamic(this, &UNButtonListViewEntry::OnButtonReleased);
}

void UNButtonListViewEntry::NativeDestruct()
{
	if (IsValid(Button))
	{
		Button->OnClicked.RemoveAll(this);
		Button->OnPressed.RemoveAll(this);
		Button->OnHovered.RemoveAll(this);
		Button->OnUnhovered.RemoveAll(this);
		Button->OnReleased.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UNButtonListViewEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	Object = nullptr;

	const UNButtonListEntry* ButtonObject = Cast<UNButtonListEntry>(ListItemObject);
	if (IsValid(ButtonObject))
	{
		SetText(ButtonObject->GetText());
		Object = ListItemObject;
	}
	OnButtonUnhovered();
}

void UNButtonListViewEntry::NativeOnEntryReleased()
{
	Object = nullptr;
}

void UNButtonListViewEntry::SetOwnerListView(UObject* Widget, UNListView* Owner)
{
	OwnerListView = Owner;
	INListViewEntry::SetOwnerListView(Widget, Owner);
}

void UNButtonListViewEntry::SetText(const FText NewText) const
{
	Text->SetText(NewText);
}

void UNButtonListViewEntry::HandleClicked()
{
	if (UNButtonListEntry* ButtonObject = Cast<UNButtonListEntry>(Object))
	{
		ButtonObject->OnPressed();
	}
}

void UNButtonListViewEntry::OnButtonPressed()
{
	if (Object == nullptr) return;
	const UNButtonListEntry* ButtonObject = Cast<UNButtonListEntry>(Object);

	if (ButtonObject != nullptr)
	{
		Button->SetBackgroundColor(FNColor::GetLinearColor(ButtonObject->PressedStateBackgroundColor));
		Text->SetColorAndOpacity(FNColor::GetLinearColor(ButtonObject->PressedStateForegroundColor));
	}
}

void UNButtonListViewEntry::OnButtonHovered()
{
	if (Object == nullptr) return;
	const UNButtonListEntry* ButtonObject = Cast<UNButtonListEntry>(Object);

	if (ButtonObject != nullptr)
	{
		Button->SetBackgroundColor(FNColor::GetLinearColor(ButtonObject->HoverStateBackgroundColor));
		Text->SetColorAndOpacity(FNColor::GetLinearColor(ButtonObject->HoverStateForegroundColor));
	}
}

void UNButtonListViewEntry::OnButtonReleased()
{
	N_VALIDATE_RETURN_VOID(LogNexusUI, Object);
	const UNButtonListEntry* ButtonObject = Cast<UNButtonListEntry>(Object);

	if (ButtonObject != nullptr)
	{
		Button->SetBackgroundColor(FNColor::GetLinearColor(ButtonObject->HoverStateBackgroundColor));
		Text->SetColorAndOpacity(FNColor::GetLinearColor(ButtonObject->HoverStateForegroundColor));
	}
}

void UNButtonListViewEntry::OnButtonUnhovered()
{
	if (Object == nullptr) return;
	const UNButtonListEntry* ButtonObject = Cast<UNButtonListEntry>(Object);

	if (ButtonObject != nullptr)
	{
		Button->SetBackgroundColor(FNColor::GetLinearColor(ButtonObject->UnhoveredStateBackgroundColor));
		Text->SetColorAndOpacity(FNColor::GetLinearColor(ButtonObject->UnhoveredStateForegroundColor));
	}
}
