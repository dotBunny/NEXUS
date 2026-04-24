// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonTextBlock.h"
#include "INListViewEntry.h"
#include "NColor.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "NTextListViewEntry.generated.h"


/**
 * Data model for a single row in a UNListView of text labels. Tracks text plus optional
 * foreground/background color overrides via the bHas* flags so unset colors fall through to
 * the entry widget's defaults.
 */
UCLASS(BlueprintType)
class NEXUSUI_API UNTextListViewEntryObject : public UObject
{
	GENERATED_BODY()

public:

	/** @return The background color override currently stored on the entry (meaningful only when HasBackgroundColor returns true). */
	UFUNCTION(BlueprintCallable, Category = "Text")
	ENColor GetBackgroundColor() const { return BackgroundColor; }

	/** @return The foreground (text) color override currently stored on the entry (meaningful only when HasForegroundColor returns true). */
	UFUNCTION(BlueprintCallable, Category = "Text")
	ENColor GetForegroundColor() const { return ForegroundColor; }

	/** @return The text the bound entry widget should display. */
	UFUNCTION(BlueprintCallable, Category = "Text")
	FText GetText() const { return Text; }

	/**
	 * Sets the background color override and marks it as present so the entry widget applies it.
	 * @param Color The ENColor value to use for the row's container background.
	 */
	UFUNCTION(BlueprintCallable, Category = "Text")
	void SetBackgroundColor(const ENColor Color)
	{
		bHasBackgroundColor = true;
		BackgroundColor = Color;
	}

	/** @return True when a background color override has been assigned; false leaves the widget's default background in place. */
	UFUNCTION(BlueprintCallable, Category = "Text")
	bool HasBackgroundColor() const
	{
		return bHasBackgroundColor;
	}

	/**
	 * Sets the foreground text color override and marks it as present so the entry widget applies it.
	 * @param Color The ENColor value to use for the row's text.
	 */
	UFUNCTION(BlueprintCallable, Category = "Text")
	void SetForegroundColor(const ENColor Color)
	{
		bHasForegroundColor = true;
		ForegroundColor = Color;
	}

	/** @return True when a foreground color override has been assigned; false leaves the widget's default text color in place. */
	UFUNCTION(BlueprintCallable, Category = "Text")
	bool HasForegroundColor() const
	{
		return bHasForegroundColor;
	}

	/**
	 * Replaces the text displayed by any entry widget bound to this data object.
	 * @param InText The text to display.
	 */
	UFUNCTION(BlueprintCallable, Category = "Text")
	void SetText(const FText& InText) { Text = InText; }

private:

	FText Text;
	bool bHasForegroundColor = false;
	bool bHasBackgroundColor = false;
	ENColor ForegroundColor = ENColor::NC_White;
	ENColor BackgroundColor = ENColor::NC_Transparent;
};


/**
 * List-view entry widget that renders a UBorder + UCommonTextBlock driven by a bound
 * UNTextListViewEntryObject. Used as a lightweight label row in UNListView controls.
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | Text ListView Entry", BlueprintType, Blueprintable, HideDropdown)
class NEXUSUI_API UNTextListViewEntry : public UUserWidget, public INListViewEntry
{
	GENERATED_BODY()

	//~INListViewEntry
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override
	{
		const UNTextListViewEntryObject* TextObject = Cast<UNTextListViewEntryObject>(ListItemObject);
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
	virtual void SetOwnerListView(UObject* Widget, UNListView* Owner) override
	{
		OwnerListView = Owner;
		Execute_OnSetOwnerListView(Widget, Owner);
	}
	//End INListViewEntry

	/**
	 * Updates the text displayed by the bound UCommonTextBlock.
	 * @param NewText The text to display in the row.
	 */
	UFUNCTION(BlueprintCallable)
	void SetText(const FText NewText) const
	{
		Text->SetText(NewText);
	}

	/**
	 * Applies a palette color to the text block's color-and-opacity.
	 * @param NewColor The ENColor palette entry used to resolve the FLinearColor applied to the text.
	 */
	UFUNCTION(BlueprintCallable)
	void SetTextColor(ENColor NewColor) const
	{
		Text->SetColorAndOpacity(FNColor::GetLinearColor(NewColor));
	}

	/**
	 * Applies a palette color to the container border's brush.
	 * @param NewColor The ENColor palette entry used to resolve the FLinearColor applied to the border.
	 */
	UFUNCTION(BlueprintCallable)
	void SetBackgroundColor(ENColor NewColor) const
	{
		Container->SetBrushColor(FNColor::GetLinearColor(NewColor));
	}

protected:
	/** Back-pointer to the list view that owns this entry; set by INListViewEntry::SetOwnerListView. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNListView> OwnerListView;

	/** Bound UBorder in the entry widget tree that provides the row background brush. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UBorder> Container;

	/** Bound UCommonTextBlock in the entry widget tree that renders the row's label. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> Text;
};