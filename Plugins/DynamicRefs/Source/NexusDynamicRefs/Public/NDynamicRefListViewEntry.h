// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once


#include "CommonTextBlock.h"
#include "INListViewEntry.h"
#include "NDynamicRefsMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/NListView.h"
#include "Macros/NValidationMacros.h"
#include "NDynamicRefListViewEntry.generated.h"

class UNDynamicRefObject;
class UCommonTextBlock;

/**
 * UMG list view entry widget for a single dynamic-reference row (reference label + nested actor list).
 */
UCLASS(ClassGroup = "NEXUS", DisplayName = "NEXUS | DynamicRef ListView Entry", BlueprintType, Blueprintable)
class NEXUSDYNAMICREFS_API UNDynamicRefListViewEntry : public UUserWidget, public INListViewEntry
{
	GENERATED_BODY()


public:
	
	virtual void SetOwnerListView(UObject* Widget, UNListView* Owner) override
	{
		OwnerListView = Owner;
		Execute_OnSetOwnerListView(Widget, Owner);
	}
	
	/**
	 * Button handler used by nested list rows to focus/select the referenced object.
	 * @param TargetObject The object clicked.
	 */
	UFUNCTION()
	void OnButtonPressed(UObject* TargetObject) const;

	/** Refresh the widget's displayed values from the bound UNDynamicRefObject. */
	UFUNCTION(BlueprintCallable, Category = "NEXUS|DynamicRefs")
	void Refresh() const;

protected:

	/** The list view that owns this entry. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNListView> OwnerListView;

	/** Label showing the reference name or ENDynamicRef display name. */
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> Reference;

	/** Nested list view of objects currently claiming the reference. */
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UNListView> References;
	
	virtual void NativeConstruct() override
	{
		Super::NativeConstruct();
		
		// Will validate it here only to throw a message in log for someone to realize they haven't hooked up the widget correctly.
		N_VALIDATE(LogNexusDynamicRefs, Reference)
		N_VALIDATE(LogNexusDynamicRefs, References)
	}
	virtual void NativeDestruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnEntryReleased() override;

private:
	/** The reference wrapper currently bound to this entry. */
	UPROPERTY()
	TObjectPtr<UNDynamicRefObject> Object;
};