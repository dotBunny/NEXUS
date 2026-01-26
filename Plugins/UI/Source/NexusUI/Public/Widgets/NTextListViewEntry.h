// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonTextBlock.h"
#include "INListViewEntry.h"
#include "NColor.h"
#include "Blueprint/UserWidget.h"
#include "Developer/NTextObject.h"
#include "NTextListViewEntry.generated.h"

class UCommonTextBlock;
class UCommonBorder;

UCLASS(ClassGroup = "NEXUS", DisplayName = "Text ListView Entry", BlueprintType, Blueprintable, HideDropdown)
class NEXUSUI_API UNTextListViewEntry : public UUserWidget, public INListViewEntry
{
	GENERATED_BODY()

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override
	{
		const UNTextObject* TextObject = Cast<UNTextObject>(ListItemObject);
		if (TextObject != nullptr)
		{
			SetText(TextObject->GetText());
			SetTextColor(TextObject->GetForegroundColor());
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
	void SetTextColor(ENColor NewColor)
	{
		Text->SetColorAndOpacity(FNColor::GetLinearColor(NewColor));
	}
	
protected:	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNListView> OwnerListView;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> Text;
};