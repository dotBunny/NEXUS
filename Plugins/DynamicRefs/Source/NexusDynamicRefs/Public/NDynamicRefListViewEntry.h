// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once


#include "INListViewEntry.h"
#include "Blueprint/UserWidget.h"
#include "NDynamicRefListViewEntry.generated.h"

class UNDynamicRefObject;
class UCommonTextBlock;

UCLASS(ClassGroup = "NEXUS", DisplayName = "DynamicRef ListView Entry", BlueprintType, Blueprintable)
class NEXUSDYNAMICREFS_API UNDynamicRefListViewEntry : public UUserWidget, public INListViewEntry
{
	GENERATED_BODY()
	
	virtual void SetOwnerListView(UObject* Widget, UNListView* Owner) override
	{
		OwnerListView = Owner;
		Execute_OnSetOwnerListView(Widget, Owner);
	}
	
	virtual void NativeDestruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnEntryReleased() override;

public:
	UFUNCTION()
	void OnButtonPressed(UObject* TargetObject) const;
	
	UFUNCTION(BlueprintCallable)
	void Refresh() const;

protected:	
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNListView> OwnerListView;

	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> Reference;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UNListView> References;
	
private:
	UPROPERTY()
	TObjectPtr<UNDynamicRefObject> Object;
};