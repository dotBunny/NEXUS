// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once

#include "CommonListView.h"
#include "INListViewEntry.h"
#include "NUIMinimal.h"
#include "NListView.generated.h"

/**
 * UCommonListView subclass that invokes INListViewEntry::SetOwnerListView on each generated
 * entry widget. Also exposes a weak ReferenceObject slot so an outer object can be stashed on
 * the list and retrieved from inside entry widgets (e.g. to bind to a view-model).
 */
UCLASS(DisplayName = "NEXUS | ListView", ClassGroup = UI, meta = (Category = "NEXUS"))
class NEXUSUI_API UNListView : public UCommonListView
{
	GENERATED_BODY()

public:
	
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override {  return NEXUS::UIEditor::PaletteCategory; }
#endif // WITH_EDITOR

	/** Stash an arbitrary outer reference on the list so entries can retrieve it during construction. */
	UFUNCTION(BlueprintCallable)
	void SetReferenceObject(UObject* Object) { ReferenceObject = Object; }

	/** @return the previously-stashed reference object, or nullptr if it has been GC'd. */
	UFUNCTION(BlueprintCallable)
	UObject* GetReferenceObject() const
	{
		if (ReferenceObject.IsValid())
		{
			return ReferenceObject.Get();
		}
		return nullptr;
	}

protected:
	//~UCommonListView
	virtual UUserWidget& OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable) override
	{
		UUserWidget& Widget =  Super::OnGenerateEntryWidgetInternal(Item, DesiredEntryClass, OwnerTable);

		if (Widget.Implements<UNListViewEntry>())
		{
			if (INListViewEntry* Entry = Cast<INListViewEntry>(&Widget);
				Entry != nullptr)
			{
				Entry->SetOwnerListView(&Widget, this);
			}
			else
			{
				INListViewEntry::Execute_OnSetOwnerListView(&Widget, this);
			}
		}
		return Widget;
	};
	//End UCommonListView

	/** Weakly-held reference object surfaced via Get/SetReferenceObject for use by entry widgets. */
	UPROPERTY(BlueprintreadOnly, Category = "NListView")
	TWeakObjectPtr<UObject> ReferenceObject = nullptr;
};
