// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "INListViewEntry.h"
#include "Blueprint/UserWidget.h"
#include "NTextListViewEntry.generated.h"

class UCommonTextBlock;
class UCommonBorder;

UCLASS(BlueprintType, Blueprintable, Experimental)
class NEXUSUI_API UNTextListViewEntry : public UUserWidget, public INListViewEntry
{
	GENERATED_BODY()

	virtual void SetOwnerListView(UObject* Widget, UNListView* Owner) override
	{
		OwnerListView = Owner;
		Execute_OnSetOwnerListView(Widget, Owner);
	}
	
protected:	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNListView> OwnerListView;
	
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> Text;
};