// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonTextBlock.h"
#include "INListViewEntry.h"
#include "NColor.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "NTextListViewEntry.generated.h"


UCLASS(BlueprintType)
class NEXUSUI_API UNTextListViewEntryObject : public UObject
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "Text")
	ENColor GetBackgroundColor() const { return BackgroundColor; }
	
	UFUNCTION(BlueprintCallable, Category = "Text")
	ENColor GetForegroundColor() const { return ForegroundColor; }
	
	UFUNCTION(BlueprintCallable, Category = "Text")
	FText GetText() const { return Text; }
	
	UFUNCTION(BlueprintCallable, Category = "Text")
	void SetBackgroundColor(const ENColor Color)
	{
		bHasBackgroundColor = true;
		BackgroundColor = Color;
	}
	
	UFUNCTION(BlueprintCallable, Category = "Text")
	bool HasBackgroundColor() const
	{
		return bHasBackgroundColor;
	}
	
	UFUNCTION(BlueprintCallable, Category = "Text")
	void SetForegroundColor(const ENColor Color)
	{
		bHasForegroundColor = true;
		ForegroundColor = Color;
	}
	
	UFUNCTION(BlueprintCallable, Category = "Text")
	bool HasForegroundColor() const
	{
		return bHasForegroundColor;
	}
	
	UFUNCTION(BlueprintCallable, Category = "Text")
	void SetText(const FText& InText) { Text = InText; }
	
private:

	FText Text;
	bool bHasForegroundColor = false;
	bool bHasBackgroundColor = false;
	ENColor ForegroundColor = ENColor::NC_White;
	ENColor BackgroundColor = ENColor::NC_Transparent;
};


UCLASS(ClassGroup = "NEXUS", DisplayName = "Text ListView Entry", BlueprintType, Blueprintable, HideDropdown)
class NEXUSUI_API UNTextListViewEntry : public UUserWidget, public INListViewEntry
{
	GENERATED_BODY()

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
	
	UFUNCTION(BlueprintCallable)
	void SetText(const FText NewText) const
	{
		Text->SetText(NewText);
	}
	
	UFUNCTION(BlueprintCallable)
	void SetTextColor(ENColor NewColor) const
	{
		Text->SetColorAndOpacity(FNColor::GetLinearColor(NewColor));
	}
	
	UFUNCTION(BlueprintCallable)
	void SetBackgroundColor(ENColor NewColor) const
	{
		Container->SetBrushColor(FNColor::GetLinearColor(NewColor));
	}

protected:	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNListView> OwnerListView;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UBorder> Container;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> Text;
};