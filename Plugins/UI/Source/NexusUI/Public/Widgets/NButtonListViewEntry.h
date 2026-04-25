// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonTextBlock.h"
#include "INListViewEntry.h"
#include "NColor.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "NButtonListViewEntry.generated.h"

/**
 * Data model for a single row in a UNListView of buttons. Carries label text, per-state color
 * slots (hover/pressed/unhovered) and a TargetObject passed back through OnPressedEvent so each
 * button can act on a specific domain object.
 */
UCLASS(BlueprintType)
class NEXUSUI_API UNButtonListViewEntryObject : public UObject
{
	GENERATED_BODY()

	/** Button-press callback signature; receives the row's TargetObject. */
	DECLARE_DELEGATE_OneParam( OnButtonPressedDelegate, UObject*);
public:

	/** @return The label text the bound button should display. */
	UFUNCTION(BlueprintCallable, Category = "Text")
	FText GetText() const { return Text; }

	/**
	 * Sets the label text displayed by any entry widget bound to this data object.
	 * @param InText The text to display.
	 */
	UFUNCTION(BlueprintCallable, Category = "Text")
	void SetText(const FText& InText) { Text = InText; }

	/**
	 * Internal click handler bound to the entry widget's button; forwards the activation to
	 * OnPressedEvent so subscribers receive the row's TargetObject.
	 */
	UFUNCTION()
	// ReSharper disable once CppMemberFunctionMayBeConst
	void OnPressed() { OnPressedEvent.ExecuteIfBound(TargetObject); };

	/** Dispatched when the bound button is clicked. Subscribers receive TargetObject for context. */
	OnButtonPressedDelegate OnPressedEvent;

	/** Text color applied to the button while the pointer is hovering over it. */
	ENColor HoverStateForegroundColor = ENColor::NC_White;
	/** Background color applied to the button while the pointer is hovering over it. */
	ENColor HoverStateBackgroundColor = ENColor::NC_BlueLight;

	/** Text color applied while the button is actively being pressed. */
	ENColor PressedStateForegroundColor = ENColor::NC_White;
	/** Background color applied while the button is actively being pressed. */
	ENColor PressedStateBackgroundColor = ENColor::NC_BlueDark;

	/** Text color applied to the idle (un-hovered) button state. */
	ENColor UnhoveredStateForegroundColor = ENColor::NC_White;
	/** Background color applied to the idle (un-hovered) button state. */
	ENColor UnhoveredStateBackgroundColor = ENColor::NC_GreyDark;

	/** Arbitrary payload forwarded to OnPressedEvent subscribers when the button is clicked. */
	UPROPERTY()
	TObjectPtr<UObject> TargetObject;
private:
	FText Text;
};


/**
 * List-view entry widget that renders a UButton + UCommonTextBlock pair driven by a bound
 * UNButtonListViewEntryObject. Swaps background/foreground colors on hover/press/release
 * using the palette slots on the bound object.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Button ListView Entry", BlueprintType, Blueprintable, HideDropdown)
class NEXUSUI_API UNButtonListViewEntry : public UUserWidget, public INListViewEntry
{
	GENERATED_BODY()

protected:
	//~UUserWidget
	virtual void NativeConstruct() override
	{
		Super::NativeConstruct();

		if (Button != nullptr)
		{
			Button->OnClicked.AddDynamic(this, &UNButtonListViewEntry::OnButtonPressed);
			Button->OnHovered.AddDynamic(this, &UNButtonListViewEntry::OnButtonHovered);
			Button->OnUnhovered.AddDynamic(this, &UNButtonListViewEntry::OnButtonUnhovered);
			Button->OnReleased.AddDynamic(this, &UNButtonListViewEntry::OnButtonReleased);
		}
	}

	virtual void NativeDestruct() override
	{
		if (Button != nullptr && Button->IsValidLowLevel())
		{
			Button->OnClicked.RemoveAll(this);
			Button->OnHovered.RemoveAll(this);
			Button->OnUnhovered.RemoveAll(this);
			Button->OnReleased.RemoveAll(this);
		}

		Super::NativeDestruct();
	}
	//End UUserWidget

	//~INListViewEntry
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override
	{
		const UNButtonListViewEntryObject* ButtonObject = Cast<UNButtonListViewEntryObject>(ListItemObject);
		if (ButtonObject != nullptr)
		{
			SetText(ButtonObject->GetText());
		}
		Button->OnClicked.AddDynamic(ButtonObject, &UNButtonListViewEntryObject::OnPressed);
		Object = ListItemObject;
		
		OnButtonUnhovered();
	}
	
	virtual void NativeOnEntryReleased() override
	{
		if (Object != nullptr)
		{
			const UNButtonListViewEntryObject* ButtonObject = Cast<UNButtonListViewEntryObject>(Object);
			Button->OnClicked.RemoveAll(ButtonObject);
			Object = nullptr;
		}
	}
public:
	virtual void SetOwnerListView(UObject* Widget, UNListView* Owner) override
	{
		OwnerListView = Owner;
		Execute_OnSetOwnerListView(Widget, Owner);
	}
	//End INListViewEntry

public:
	/**
	 * Updates the label text displayed on the bound button's child text block.
	 * @param NewText The text to display on the button.
	 */
	UFUNCTION(BlueprintCallable)
	void SetText(const FText NewText) const
	{
		Text->SetText(NewText);
	}

protected:

	/** Applies the bound entry object's pressed-state colors to the button/text. */
	UFUNCTION()
	void OnButtonPressed()
	{
		if (Object == nullptr) return;
		const UNButtonListViewEntryObject* ButtonObject = Cast<UNButtonListViewEntryObject>(Object);

		if (ButtonObject != nullptr)
		{
			Button->SetBackgroundColor(FNColor::GetLinearColor(ButtonObject->PressedStateBackgroundColor));
			Text->SetColorAndOpacity(FNColor::GetLinearColor(ButtonObject->PressedStateForegroundColor));
		}
	}
	
	/** Applies the bound entry object's hover-state colors to the button/text. */
	UFUNCTION()
	void OnButtonHovered()
	{
		if (Object == nullptr) return;
		const UNButtonListViewEntryObject* ButtonObject = Cast<UNButtonListViewEntryObject>(Object);

		if (ButtonObject != nullptr)
		{
			Button->SetBackgroundColor(FNColor::GetLinearColor(ButtonObject->HoverStateBackgroundColor));
			Text->SetColorAndOpacity(FNColor::GetLinearColor(ButtonObject->HoverStateForegroundColor));
		}
	}
	
	/** Applies the bound entry object's hover-state colors on mouse release (pointer is still over the button). */
	UFUNCTION()
	void OnButtonReleased()
	{
		if (Object == nullptr) return;
		const UNButtonListViewEntryObject* ButtonObject = Cast<UNButtonListViewEntryObject>(Object);

		if (ButtonObject != nullptr)
		{
			Button->SetBackgroundColor(FNColor::GetLinearColor(ButtonObject->HoverStateBackgroundColor));
			Text->SetColorAndOpacity(FNColor::GetLinearColor(ButtonObject->HoverStateForegroundColor));
		}
	}
	
	/** Applies the bound entry object's idle-state colors when the pointer leaves the button. */
	UFUNCTION()
	void OnButtonUnhovered()
	{
		if (Object == nullptr) return;
		const UNButtonListViewEntryObject* ButtonObject = Cast<UNButtonListViewEntryObject>(Object);

		if (ButtonObject != nullptr)
		{
			Button->SetBackgroundColor(FNColor::GetLinearColor(ButtonObject->UnhoveredStateBackgroundColor));
			Text->SetColorAndOpacity(FNColor::GetLinearColor(ButtonObject->UnhoveredStateForegroundColor));
		}
	}
	
	/** Back-pointer to the list view that owns this entry; set by INListViewEntry::SetOwnerListView. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNListView> OwnerListView;

	/** Bound UButton in the entry widget tree that surfaces click/hover events. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UButton> Button;

	/** Bound UCommonTextBlock in the entry widget tree that renders the button's label. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> Text;

private:

	/** The currently bound UNButtonListViewEntryObject backing this row; nulled when the entry is released. */
	UPROPERTY()
	TObjectPtr<UObject> Object;
};