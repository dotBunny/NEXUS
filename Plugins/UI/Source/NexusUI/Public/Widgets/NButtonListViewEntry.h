// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "INListViewEntry.h"
#include "NColor.h"
#include "Blueprint/UserWidget.h"
#include "NUIMinimal.h"
#include "NButtonListViewEntry.generated.h"

class UButton;
class UCommonTextBlock;

/**
 * Data model for a single row in a UNListView of buttons. Carries label text, per-state color
 * slots (hover/pressed/unhovered) and a TargetObject passed back through OnPressedEvent so each
 * button can act on a specific domain object.
 */
UCLASS(BlueprintType, DisplayName = "NEXUS | Button List Entry")
class NEXUSUI_API UNButtonListEntry : public UObject
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
 * UNButtonListEntry. Swaps background/foreground colors on hover/press/release
 * using the palette slots on the bound object.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Button ListView Entry", BlueprintType, Blueprintable, HideDropdown)
class NEXUSUI_API UNButtonListViewEntry : public UUserWidget, public INListViewEntry
{
	GENERATED_BODY()

protected:
	//~UUserWidget
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//End UUserWidget

	//~INListViewEntry
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnEntryReleased() override;
public:
	virtual void SetOwnerListView(UObject* Widget, UNListView* Owner) override;
	//End INListViewEntry

public:
	/**
	 * Updates the label text displayed on the bound button's child text block.
	 * @param NewText The text to display on the button.
	 */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|User Interface")
	void SetText(const FText NewText) const;

protected:

	/** Forwards a completed click on the bound button to the current entry object's OnPressedEvent. */
	UFUNCTION()
	void HandleClicked();

	/** Applies the bound entry object's pressed-state colors to the button/text. */
	UFUNCTION()
	void OnButtonPressed();

	/** Applies the bound entry object's hover-state colors to the button/text. */
	UFUNCTION()
	void OnButtonHovered();

	/** Applies the bound entry object's hover-state colors on mouse release (pointer is still over the button). */
	UFUNCTION()
	void OnButtonReleased();

	/** Applies the bound entry object's idle-state colors when the pointer leaves the button. */
	UFUNCTION()
	void OnButtonUnhovered();

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

	/** The currently bound UNButtonListEntry backing this row; nulled when the entry is released. */
	UPROPERTY()
	TObjectPtr<UObject> Object;
};
